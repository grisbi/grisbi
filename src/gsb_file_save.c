/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)	                      */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                 */
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

/**
 * \file gsb_file_save.c
 * save the file
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

/*START_INCLUDE*/
#include "gsb_file_save.h"
#include "bet_data.h"
#include "bet_data_finance.h"
#include "bet_graph.h"
#include "bet_tab.h"
#include "custom_list.h"
#include "dialog.h"
#include "grisbi_app.h"
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
#include "gsb_locale.h"
#include "gsb_real.h"
#include "gsb_rgba.h"
#include "gsb_select_icon.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "import_csv.h"
#include "navigation.h"
#include "structures.h"
#include "utils_dates.h"
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

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * save the account part
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 **/
static gulong gsb_file_save_account_part (gulong iterator,
										  gulong *length_calculated,
										  gchar **file_content)
{
	GSList *list_tmp;

	list_tmp = gsb_data_account_get_list_accounts ();

	while (list_tmp)
	{
		gchar *string_to_free1;
		gchar *string_to_free2;
		gint account_number;
		gint j;
		gint floating_point;
		gchar *sort_list;
		gchar *sort_kind_column;
		GSList *list_tmp_2;
		const gchar *account_icon_name;
		gchar *icon_name;
		gchar *new_string;
		gchar *init_balance;
		gchar *mini_wanted;
		gchar *mini_auto;
		gchar *owner_str;
		gchar *bet_str;
		gchar *tmp_str;
		KindAccount kind;
		GrisbiWinEtat *w_etat;

		account_number = gsb_data_account_get_no_account (list_tmp->data);

		/* set the sort_list */
		list_tmp_2 = gsb_data_account_get_sort_list (account_number);
		sort_list = NULL;

		while (list_tmp_2)
		{
			if (sort_list)
			{
				string_to_free1 = sort_list;
				string_to_free2 = utils_str_itoa (GPOINTER_TO_INT (list_tmp_2->data));
				sort_list = g_strconcat (string_to_free1, "/", string_to_free2, NULL);

				g_free (string_to_free1);
				g_free (string_to_free2);
			}
			else
				sort_list = utils_str_itoa (GPOINTER_TO_INT (list_tmp_2->data));

			list_tmp_2 = list_tmp_2->next;
		}

		/* set the default sort kind for the columns */
		sort_kind_column = NULL;

		for (j=0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
		{
			if (sort_kind_column)
			{
				string_to_free1 = sort_kind_column;
				string_to_free2 = utils_str_itoa (gsb_data_account_get_element_sort (account_number, j));
				sort_kind_column = g_strconcat (string_to_free1, "-", string_to_free2, NULL);

				g_free (string_to_free1);
				g_free (string_to_free2);
			}
			else
				sort_kind_column = utils_str_itoa (gsb_data_account_get_element_sort (account_number, j));
		}

		/* set the reals */
		floating_point = gsb_data_account_get_currency_floating_point (account_number);
		init_balance = gsb_real_safe_real_to_string (gsb_data_account_get_init_balance (account_number, -1),
													 floating_point);
		mini_wanted = gsb_real_safe_real_to_string (gsb_data_account_get_mini_balance_wanted (account_number),
													floating_point);
		mini_auto = gsb_real_safe_real_to_string (gsb_data_account_get_mini_balance_authorized (account_number),
												  floating_point);

		/* protect the owner adress and comment */
		owner_str = utils_str_protect_unprotect_multilines_text (gsb_data_account_get_holder_address
																 (account_number),
																 TRUE);
		tmp_str = utils_str_protect_unprotect_multilines_text (gsb_data_account_get_comment (account_number),
															   TRUE);

		kind = gsb_data_account_get_kind (account_number);

		/* get basename of icon if necessary */
		account_icon_name = gsb_data_account_get_name_icon (account_number);
		w_etat = grisbi_win_get_w_etat ();
		if (w_etat->use_icons_file_dir)
		{
			if (account_icon_name == NULL)
				icon_name = NULL;
			else
				icon_name = g_path_get_basename (account_icon_name);
		}
		else
		{
			if (account_icon_name == NULL)
				icon_name = NULL;
			else
				icon_name = g_strdup (account_icon_name);
		}

		/* now we can fill the file content */
		string_to_free1 = g_markup_printf_escaped ("\t<Account\n"
														"\t\tName=\"%s\"\n"
														"\t\tId=\"%s\"\n"
														"\t\tNumber=\"%d\"\n"
														"\t\tOwner=\"%s\"\n"
														"\t\tKind=\"%d\"\n"
														"\t\tCurrency=\"%d\"\n"
														"\t\tPath_icon=\"%s\"\n"
														"\t\tBank=\"%d\"\n"
														"\t\tBank_branch_code=\"%s\"\n"
														"\t\tBank_account_number=\"%s\"\n"
														"\t\tKey=\"%s\"\n"
														"\t\tBank_account_IBAN=\"%s\"\n"
														"\t\tInitial_balance=\"%s\"\n"
														"\t\tMinimum_wanted_balance=\"%s\"\n"
														"\t\tMinimum_authorised_balance=\"%s\"\n"
														"\t\tClosed_account=\"%d\"\n"
														"\t\tShow_marked=\"%d\"\n"
														"\t\tShow_archives_lines=\"%d\"\n"
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
														"\t\tBet_use_budget=\"%d\"",
														my_safe_null_str(gsb_data_account_get_name (account_number)),
														my_safe_null_str(gsb_data_account_get_id (account_number)),
														account_number,
														my_safe_null_str(gsb_data_account_get_holder_name
																		 (account_number)),
														kind,
														gsb_data_account_get_currency (account_number),
														my_safe_null_str (icon_name),
														gsb_data_account_get_bank (account_number),
														my_safe_null_str (gsb_data_account_get_bank_branch_code
																		  (account_number)),
														my_safe_null_str (gsb_data_account_get_bank_account_number
																		  (account_number)),
														my_safe_null_str (gsb_data_account_get_bank_account_key
																		  (account_number)),
														my_safe_null_str (gsb_data_account_get_bank_account_iban
																		  (account_number)),
														my_safe_null_str (init_balance),
														my_safe_null_str (mini_wanted),
														my_safe_null_str (mini_auto),
														gsb_data_account_get_closed_account (account_number),
														gsb_data_account_get_r (account_number),
														gsb_data_account_get_l (account_number),
														gsb_data_account_get_nb_rows (account_number),
														my_safe_null_str(tmp_str),
														my_safe_null_str (owner_str),
														gsb_data_account_get_default_debit (account_number),
														gsb_data_account_get_default_credit (account_number),
														gsb_data_account_get_reconcile_sort_type (account_number),
														gsb_data_account_get_split_neutral_payment (account_number),
														my_safe_null_str(sort_list),
														gsb_data_account_get_sort_type (account_number),
														gsb_data_account_get_sort_column (account_number),
														my_safe_null_str(sort_kind_column),
														gsb_data_account_get_bet_use_budget (account_number));

		if (gsb_data_account_get_bet_use_budget (account_number) > 0)
		{
			BetTypeOnglets bet_show_onglets;

			bet_show_onglets = gsb_data_account_get_bet_show_onglets (account_number);

			switch (bet_show_onglets)
			{
				case BET_ONGLETS_HIST:
					bet_str = g_markup_printf_escaped ("\t\tBet_credit_card=\"%d\"\n"
													   "\t\tBet_SD=\"%d\"\n"
													   "\t\tBet_Fi=\"%d\" />\n",
													   gsb_data_account_get_bet_credit_card (account_number),
													   gsb_data_account_get_bet_hist_data (account_number),
													   gsb_data_account_get_bet_hist_fyear (account_number));
					new_string = g_strconcat (string_to_free1, "\n", bet_str, NULL);
					g_free (bet_str);
					g_free (string_to_free1);
					break;
				case BET_ONGLETS_CAP:
					bet_str = g_markup_printf_escaped ("\t\tBet_credit_card=\"%d\" />\n",
													   gsb_data_account_get_bet_credit_card (account_number));
					new_string = g_strconcat (string_to_free1, "\n", bet_str, NULL);
					g_free (bet_str);
					g_free (string_to_free1);
					break;
				case BET_ONGLETS_ASSET:
				case BET_ONGLETS_SANS:
					new_string = g_strconcat (string_to_free1, " />\n", NULL);
					g_free (string_to_free1);
					break;
				case BET_ONGLETS_PREV:
				default:
					string_to_free2 = gsb_format_gdate_safe (gsb_data_account_get_bet_start_date
																   (account_number));
					bet_str = g_markup_printf_escaped ("\t\tBet_credit_card=\"%d\"\n"
													   "\t\tBet_start_date=\"%s\"\n"
													   "\t\tBet_months=\"%d\"\n"
													   "\t\tBet_UT=\"%d\"\n"
													   "\t\tBet_auto_inc_month=\"%d\"\n"
													   "\t\tBet_select_transaction_label=\"%d\"\n"
													   "\t\tBet_select_scheduled_label=\"%d\"\n"
													   "\t\tBet_select_futur_label=\"%d\"\n"
													   "\t\tBet_SD=\"%d\"\n"
													   "\t\tBet_Fi=\"%d\" />\n",
													   gsb_data_account_get_bet_credit_card (account_number),
													   my_safe_null_str (string_to_free2),
													   gsb_data_account_get_bet_months (account_number),
													   gsb_data_account_get_bet_spin_range (account_number),
													   gsb_data_account_get_bet_auto_inc_month (account_number),
													   gsb_data_account_get_bet_select_label (account_number,
																							  SPP_ORIGIN_TRANSACTION),
													   gsb_data_account_get_bet_select_label (account_number,
																							  SPP_ORIGIN_SCHEDULED),
													   gsb_data_account_get_bet_select_label (account_number,
																							  SPP_ORIGIN_FUTURE),
													   gsb_data_account_get_bet_hist_data (account_number),
													   gsb_data_account_get_bet_hist_fyear (account_number));
					new_string = g_strconcat (string_to_free1, "\n", bet_str, NULL);
					g_free (bet_str);
					g_free (string_to_free1);
					g_free (string_to_free2);
					break;
			}
		}
		else
		{
			new_string = g_strconcat (string_to_free1, " />\n", NULL);
			g_free (string_to_free1);
		}

		g_free (sort_list);
		g_free (sort_kind_column);
		g_free (init_balance);
		if (icon_name)
			g_free (icon_name);
		g_free (mini_auto);
		g_free (mini_wanted);
		g_free (owner_str);
		g_free (tmp_str);

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
	}

	/* and return the new iterator */
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
 **/
static gulong gsb_file_save_archive_part (gulong iterator,
										  gulong *length_calculated,
										  gchar **file_content)
{
    GSList *list_tmp;

    list_tmp = gsb_data_archive_get_archives_list ();

    while (list_tmp)
    {
		gint archive_number;
		gchar *new_string;
		gchar *beginning_date;
		gchar *end_date;

		archive_number = gsb_data_archive_get_no_archive (list_tmp->data);

		/* set the date */
		beginning_date = gsb_format_gdate_safe (gsb_data_archive_get_beginning_date (archive_number));
		end_date = gsb_format_gdate_safe (gsb_data_archive_get_end_date (archive_number));

		/* now we can fill the file content */
		new_string = g_markup_printf_escaped ("\t<Archive Nb=\"%d\" Na=\"%s\" Bdte=\"%s\" "
											  "Edte=\"%s\" Fye=\"%d\" Rep=\"%s\" />\n",
											  archive_number,
											  my_safe_null_str(gsb_data_archive_get_name (archive_number)),
											  my_safe_null_str(beginning_date),
											  my_safe_null_str(end_date),
											  gsb_data_archive_get_fyear (archive_number),
											  my_safe_null_str(gsb_data_archive_get_report_title (archive_number)));

		if (beginning_date)
			g_free (beginning_date);
		if (end_date)
			g_free (end_date);

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
    }

	/* and return the new iterator */
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
 **/
static gulong gsb_file_save_bank_part (gulong iterator,
									   gulong *length_calculated,
									   gchar **file_content)
{
    GSList *list_tmp;

    list_tmp = gsb_data_bank_get_bank_list ();

    while (list_tmp)
    {
		gint bank_number;
		gchar *new_string;
		gchar *adr_str;
		gchar *rem_str;

		bank_number = gsb_data_bank_get_no_bank (list_tmp->data);

		/* protect adr_common and rem */
		adr_str = utils_str_protect_unprotect_multilines_text (gsb_data_bank_get_bank_address (bank_number), TRUE);
		rem_str = utils_str_protect_unprotect_multilines_text (gsb_data_bank_get_bank_note (bank_number), TRUE);

		/* now we can fill the file content */
		new_string = g_markup_printf_escaped ("\t<Bank Nb=\"%d\" Na=\"%s\" Co=\"%s\" BIC=\"%s\" "
											  "Adr=\"%s\" Tel=\"%s\" Mail=\"%s\" Web=\"%s\" Nac=\"%s\" Faxc=\"%s\" "
											  "Telc=\"%s\" Mailc=\"%s\" Rem=\"%s\" />\n",
											  bank_number,
											  my_safe_null_str(gsb_data_bank_get_name (bank_number)),
											  my_safe_null_str(gsb_data_bank_get_code (bank_number)),
											  my_safe_null_str(gsb_data_bank_get_bic (bank_number)),
											  my_safe_null_str(adr_str),
											  my_safe_null_str(gsb_data_bank_get_bank_tel (bank_number)),
											  my_safe_null_str(gsb_data_bank_get_bank_mail (bank_number)),
											  my_safe_null_str(gsb_data_bank_get_bank_web (bank_number)),
											  my_safe_null_str(gsb_data_bank_get_correspondent_name (bank_number)),
											  my_safe_null_str(gsb_data_bank_get_correspondent_fax (bank_number)),
											  my_safe_null_str(gsb_data_bank_get_correspondent_tel (bank_number)),
											  my_safe_null_str(gsb_data_bank_get_correspondent_mail (bank_number)),
											  my_safe_null_str(rem_str));

		g_free (adr_str);
		g_free (rem_str);

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
    }

	/* and return the new iterator */
	return iterator;
}

/**
 * save the balance estimate part
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 **/
static gulong gsb_file_save_bet_part (gulong iterator,
									  gulong *length_calculated,
									  gchar **file_content)
{
	gchar *new_string;
	GPtrArray *tab;
	gint i;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

	/* save the general informations */
	new_string = g_markup_printf_escaped ("\t<Bet Ddte=\"%d\" Bet_deb_cash_account_option=\"%d\"/>\n",
										  w_etat->bet_debut_period, w_etat->bet_cash_account_option);

	/* append the new string to the file content */
	iterator = gsb_file_save_append_part (iterator,
										  length_calculated,
										  file_content,
										  new_string);

	tab = bet_data_get_strings_to_save ();

	if (tab == NULL)
		return iterator;

	for (i = 0; i < (gint) tab->len; i++)
	{
		new_string = g_ptr_array_index (tab, i);

		/* append the new string to the file content and take the new iterator */
		iterator =  gsb_file_save_append_part (iterator,
											   length_calculated,
											   file_content,
											   new_string);
	}

	/* free the tab */
	g_ptr_array_free (tab, TRUE);

	/* and return the new iterator */
	return iterator;
}

#ifdef HAVE_GOFFICE
/**
 * save the bet graph preferences part
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 **/
static gulong gsb_file_save_bet_graph_part (gulong iterator,
											gulong *length_calculated,
											gchar **file_content)
{
	gchar *new_string = NULL;

	/* save the forecast preferences */
	new_string = bet_graph_get_configuration_string (BET_ONGLETS_PREV);

	/* append the new string to the file content */
	iterator = gsb_file_save_append_part (iterator,
										  length_calculated,
										  file_content,
										  new_string);

	/* save the historical preferences */
	new_string = bet_graph_get_configuration_string (BET_ONGLETS_HIST);

	/* append the new string to the file content */
	iterator = gsb_file_save_append_part (iterator,
										  length_calculated,
										  file_content,
										  new_string);

	/* and return the new iterator */
	return iterator;
}

#endif /* HAVE_GOFFICE */
/**
 * save the currency_links
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 **/
static gulong gsb_file_save_currency_link_part (gulong iterator,
												gulong *length_calculated,
												gchar **file_content)
{
    GSList *list_tmp;

    list_tmp = gsb_data_currency_link_get_currency_link_list ();

    while (list_tmp)
    {
		gchar *new_string;
		gint link_number;
		gchar *change_rate;
		gchar *str_date;

		link_number = gsb_data_currency_link_get_no_currency_link (list_tmp->data);

		/* set the number */
		change_rate = gsb_real_safe_real_to_string (
							gsb_data_currency_link_get_change_rate (link_number), -1);

		/* set the date of modification */
		str_date = gsb_format_gdate_safe (gsb_data_currency_link_get_modified_date (link_number));

		/* now we can fill the file content */
		new_string = g_markup_printf_escaped ("\t<Currency_link Nb=\"%d\" Cu1=\"%d\" Cu2=\"%d\" "
											  "Ex=\"%s\" Modified_date=\"%s\" Fl=\"%d\" />\n",
											  link_number,
											  gsb_data_currency_link_get_first_currency (link_number),
											  gsb_data_currency_link_get_second_currency (link_number),
											  my_safe_null_str (change_rate),
											  str_date,
											  gsb_data_currency_link_get_fixed_link (link_number));

		g_free (change_rate);
		g_free (str_date);

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
    }

	/* and return the new iterator */
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
 **/
static gulong gsb_file_save_currency_part (gulong iterator,
										   gulong *length_calculated,
										   gchar **file_content)
{
    GSList *list_tmp;

    list_tmp = gsb_data_currency_get_currency_list ();

    while (list_tmp)
    {
		gchar *new_string;
		gint currency_number;

		currency_number = gsb_data_currency_get_no_currency (list_tmp->data);

		/* now we can fill the file content */

		new_string = g_markup_printf_escaped ("\t<Currency Nb=\"%d\" Na=\"%s\" Co=\"%s\" "
											  "Ico=\"%s\" Fl=\"%d\" />\n",
											  currency_number,
											  my_safe_null_str(gsb_data_currency_get_name (currency_number)),
											  my_safe_null_str(gsb_data_currency_get_nickname (currency_number)),
											  my_safe_null_str(gsb_data_currency_get_code_iso4217 (currency_number)),
											  gsb_data_currency_get_floating_point (currency_number));

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
    }

	/* and return the new iterator */
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
 **/
static gulong gsb_file_save_financial_year_part (gulong iterator,
												 gulong *length_calculated,
												 gchar **file_content)
{
    GSList *list_tmp;

    list_tmp = gsb_data_fyear_get_fyears_list ();

    while (list_tmp)
    {
		gchar *new_string;
		gint fyear_number;
		gchar *beginning_date;
		gchar *end_date;

		fyear_number = gsb_data_fyear_get_no_fyear (list_tmp->data);

		/* set the date */
		beginning_date = gsb_format_gdate_safe (gsb_data_fyear_get_beginning_date(fyear_number));
		end_date = gsb_format_gdate_safe (gsb_data_fyear_get_end_date(fyear_number));

		/* now we can fill the file content */
		new_string = g_markup_printf_escaped ("\t<Financial_year Nb=\"%d\" Na=\"%s\" Bdte=\"%s\" "
											  "Edte=\"%s\" Sho=\"%d\" />\n",
											  fyear_number,
											  my_safe_null_str(gsb_data_fyear_get_name (fyear_number)),
											  my_safe_null_str(beginning_date),
											  my_safe_null_str(end_date),
											  gsb_data_fyear_get_form_show (fyear_number));

		g_free (beginning_date);
		g_free (end_date);

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
    }

	/* and return the new iterator */
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
 **/
static gulong gsb_file_save_general_part (gulong iterator,
										  gulong *length_calculated,
										  gchar **file_content,
										  gint archive_number)
{
	GQueue *tmp_queue;
	gchar *adr_common_str = NULL;
	gchar *adr_secondary_str = NULL;
	gchar *string_to_free1;
	gchar *string_to_free2;
	gchar *string_to_free3;
	gint i;
	gint j;
	gint k;
	gchar *transactions_view;
	gchar *scheduler_column_width_write;
	gchar *transaction_column_width_write;
	gchar *transaction_column_align_write;
	gchar *new_string;
	gchar *bet_array_column_width_write;
	gchar *date_format;
	gchar *mon_decimal_point;
	gchar *mon_thousands_sep;
	gchar *navigation_order_list = NULL;
	gchar *form_organization;
	gchar *form_columns_width;
	gboolean is_archive = FALSE;
	GrisbiWinEtat *w_etat;
	GrisbiWinRun *w_run;

	/* prepare stuff to save general information */
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* prepare transactions_view */
	transactions_view = gsb_transactions_list_get_tab_affichage_ope_to_string ();

	/* prepare transaction_column_width_write */
	transaction_column_width_write = gsb_transactions_list_get_tab_width_col_treeview_to_string ();

	/* prepare scheduler_column_width_write */
	scheduler_column_width_write = gsb_scheduler_list_get_largeur_col_treeview_to_string ();

	/* prepare transaction_column_align_write */
	transaction_column_align_write = gsb_transactions_list_get_tab_align_col_treeview_to_string ();

	/* prépare l'ordre des pages dans le panneau de gauche */
	tmp_queue = gsb_gui_navigation_get_pages_list ();

	for (i = 0 ; i < (gint) tmp_queue->length ; i++)
	{
		GsbGuiNavigationPage *page;

		page = g_queue_peek_nth (tmp_queue, i);

		if (navigation_order_list == NULL)
			navigation_order_list = utils_str_itoa (page->type_page);
		else
		{
			navigation_order_list = g_strconcat (string_to_free1 =  navigation_order_list,
												 "-",
												 string_to_free2 = utils_str_itoa (page->type_page),
												 NULL);

			g_free (string_to_free1);
			g_free (string_to_free2);
		}
	}

	/* set the form organization */
	form_organization = NULL;

	for (k=0 ; k<MAX_HEIGHT ; k++)
	{
		for (j=0 ; j<MAX_WIDTH ; j++)
		{
			if (form_organization)
			{
				form_organization = g_strconcat (string_to_free1 = form_organization,
												 "-",
												 string_to_free2 = utils_str_itoa (gsb_data_form_get_value (j, k)),
												 NULL);
				g_free (string_to_free1);
				g_free (string_to_free2);
			}
			else
				form_organization = utils_str_itoa (gsb_data_form_get_value (j, k));
		}
	}

	/* set the form columns width */
	form_columns_width = NULL;

	for (k=0 ; k<MAX_WIDTH ; k++)
	{
		if (form_columns_width)
		{
			form_columns_width = g_strconcat (string_to_free1 = form_columns_width,
											  "-",
											  string_to_free2 = utils_str_itoa (gsb_data_form_get_width_column (k)),
											  NULL);
			g_free (string_to_free1);
			g_free (string_to_free2);
		}
		else
			form_columns_width = utils_str_itoa (gsb_data_form_get_width_column (k));
	}

	/* prepare bet_array_column_width_write */
	bet_array_column_width_write = bet_array_get_largeur_col_treeview_to_string ();

	/* save localization data */
	date_format = gsb_date_get_format_date ();
	mon_decimal_point = gsb_locale_get_mon_decimal_point ();
	mon_thousands_sep = gsb_locale_get_mon_thousands_sep ();
	if (mon_thousands_sep == NULL)
		mon_thousands_sep = g_strdup ("empty");

	/* if we save an archive, we save it here */
	if (archive_number || w_etat->is_archive)
		is_archive = TRUE;

	/* protect adr_common */
	adr_common_str = utils_str_protect_unprotect_multilines_text (w_etat->adr_common, TRUE);

	/* protect adr_secondary */
	adr_secondary_str = utils_str_protect_unprotect_multilines_text (w_etat->adr_secondary, TRUE);

	/* save the general information */
	string_to_free1 = utils_str_dtostr (etat.bet_capital,
											 gsb_data_currency_get_floating_point (etat.bet_currency),
											 TRUE);
	string_to_free2 = utils_str_dtostr (etat.bet_taux_annuel, BET_TAUX_DIGITS, TRUE);
	string_to_free3 = utils_str_dtostr (etat.bet_frais, BET_TAUX_DIGITS, TRUE);
	new_string = g_markup_printf_escaped ("\t<General\n"
										  "\t\tFile_version=\"%s\"\n"
										  "\t\tGrisbi_version=\"%s\"\n"
										  "\t\tCrypt_file=\"%d\"\n"
										  "\t\tArchive_file=\"%d\"\n"
										  "\t\tFile_title=\"%s\"\n"
										  "\t\tUse_icons_file_dir=\"%d\"\n"
										  "\t\tGeneral_address=\"%s\"\n"
										  "\t\tSecond_general_address=\"%s\"\n"
										  "\t\tDate_format=\"%s\"\n"
										  "\t\tDecimal_point=\"%s\"\n"
										  "\t\tThousands_separator=\"%s\"\n"
										  "\t\tParty_list_currency_number=\"%d\"\n"
										  "\t\tCategory_list_currency_number=\"%d\"\n"
										  "\t\tBudget_list_currency_number=\"%d\"\n"
										  "\t\tNavigation_list_order=\"%s\"\n"
										  "\t\tScheduler_view=\"%d\"\n"
										  "\t\tScheduler_custom_number=\"%d\"\n"
										  "\t\tScheduler_custom_menu=\"%d\"\n"
										  "\t\tScheduler_set_default_account=\"%d\"\n"
										  "\t\tScheduler_default_account_number=\"%d\"\n"
										  "\t\tScheduler_set_fixed_date=\"%d\"\n"
										  "\t\tScheduler_default_fixed_date=\"%d\"\n"
										  "\t\tImport_interval_search=\"%d\"\n"
										  "\t\tImport_copy_payee_in_note=\"%d\"\n"
										  "\t\tImport_extract_number_for_check=\"%d\"\n"
										  "\t\tImport_fusion_transactions=\"%d\"\n"
										  "\t\tImport_categorie_for_payee=\"%d\"\n"
										  "\t\tImport_fyear_by_value_date=\"%d\"\n"
										  "\t\tImport_qif_no_import_categories=\"%d\"\n"
										  "\t\tImport_qif_use_field_extract_method_payment=\"%d\"\n"
										  "\t\tExport_file_format=\"%d\"\n"
										  "\t\tExport_files_traitement=\"%d\"\n"
										  "\t\tExport_force_US_dates=\"%d\"\n"
										  "\t\tExport_force_US_numbers=\"%d\"\n"
										  "\t\tExport_quote_dates=\"%d\"\n"
										  "\t\tForm_date_force_prev_year=\"%d\"\n"
										  "\t\tForm_columns_number=\"%d\"\n"
										  "\t\tForm_lines_number=\"%d\"\n"
										  "\t\tForm_organization=\"%s\"\n"
										  "\t\tForm_columns_width=\"%s\"\n"
										  "\t\tReconcile_end_date=\"%d\"\n"
										  "\t\tReconcile_sort=\"%d\"\n"
										  "\t\tUse_logo=\"%d\"\n"
										  "\t\tName_logo=\"%s\"\n"
										  "\t\tRemind_display_per_account=\"%d\"\n"
										  "\t\tTransactions_view=\"%s\"\n"
										  "\t\tTwo_lines_showed=\"%d\"\n"
										  "\t\tThree_lines_showed=\"%d\"\n"
										  "\t\tTransaction_column_width=\"%s\"\n"
										  "\t\tTransaction_column_align=\"%s\"\n"
										  "\t\tScheduler_column_width=\"%s\"\n"
										  "\t\tCombofix_mixed_sort=\"%d\"\n"
										  "\t\tCombofix_case_sensitive=\"%d\"\n"
										  "\t\tCombofix_force_payee=\"%d\"\n"
										  "\t\tCombofix_force_category=\"%d\"\n"
										  "\t\tAutomatic_amount_separator=\"%d\"\n"
										  "\t\tCSV_separator=\"%s\"\n"
										  "\t\tCSV_force_date_valeur_with_date=\"%d\"\n"
										  "\t\tMetatree_assoc_mode=\"%d\"\n"
										  "\t\tMetatree_sort_transactions=\"%d\"\n"
										  "\t\tMetatree_unarchived_payees=\"%d\"\n"
										  "\t\tAdd_archive_in_total_balance=\"%d\"\n"
										  "\t\tBet_array_column_width=\"%s\"\n"
										  "\t\tBet_capital=\"%s\"\n"
										  "\t\tBet_currency=\"%d\"\n"
										  "\t\tBet_taux_annuel=\"%s\"\n"
										  "\t\tBet_index_duree=\"%d\"\n"
										  "\t\tBet_frais=\"%s\"\n"
										  "\t\tBet_type_taux=\"%d\" />\n",
										  my_safe_null_str(VERSION_FICHIER),
										  my_safe_null_str(VERSION),
										  w_etat->crypt_file,
										  is_archive,
										  my_safe_null_str (w_etat->accounting_entity),
										  w_etat->use_icons_file_dir,
										  my_safe_null_str(adr_common_str),
										  my_safe_null_str(adr_secondary_str),
										  my_safe_null_str (date_format),
										  my_safe_null_str (mon_decimal_point),
										  my_safe_null_str (mon_thousands_sep),
										  w_etat->no_devise_totaux_tiers,
										  w_etat->no_devise_totaux_categ,
										  w_etat->no_devise_totaux_ib,
										  my_safe_null_str (navigation_order_list),
										  etat.affichage_echeances,
										  etat.affichage_echeances_perso_nb_libre,
										  etat.affichage_echeances_perso_j_m_a,
										  etat.scheduler_set_default_account,
										  etat.scheduler_default_account_number,
										  etat.scheduler_set_fixed_date,
										  etat.scheduler_set_fixed_date_day,
										  etat.import_files_nb_days,
										  etat.copy_payee_in_note,
										  etat.extract_number_for_check,
										  etat.fusion_import_transactions,
										  etat.associate_categorie_for_payee,
										  etat.get_fyear_by_value_date,
										  etat.qif_no_import_categories,
										  etat.qif_use_field_extract_method_payment,
										  etat.export_file_format,
										  etat.export_files_traitement,
										  etat.export_force_US_dates,
										  etat.export_force_US_numbers,
										  w_etat->export_quote_dates,
										  w_etat->form_date_force_prev_year,
										  gsb_data_form_get_nb_columns (),
										  gsb_data_form_get_nb_rows (),
										  my_safe_null_str(form_organization),
										  my_safe_null_str(form_columns_width),
										  w_etat->reconcile_end_date,
										  w_etat->reconcile_sort,
										  etat.utilise_logo,
										  my_safe_null_str(etat.name_logo),
										  w_etat->retient_affichage_par_compte,
										  my_safe_null_str(transactions_view),
										  w_run->display_two_lines,
										  w_run->display_three_lines,
										  my_safe_null_str(transaction_column_width_write),
										  my_safe_null_str (transaction_column_align_write),
										  my_safe_null_str(scheduler_column_width_write),
										  etat.combofix_mixed_sort,
										  etat.combofix_case_sensitive,
										  etat.combofix_force_payee,
										  etat.combofix_force_category,
										  etat.automatic_separator,
										  my_safe_null_str(etat.csv_separator),
										  etat.csv_force_date_valeur_with_date,
										  w_etat->metatree_assoc_mode,
										  w_etat->metatree_sort_transactions,
										  w_etat->metatree_unarchived_payees,
										  w_etat->metatree_add_archive_in_totals,
										  my_safe_null_str (bet_array_column_width_write),
										  my_safe_null_str (string_to_free1),
										  etat.bet_currency,
										  my_safe_null_str (string_to_free2),
										  etat.bet_index_duree,
										  my_safe_null_str (string_to_free3),
										  etat.bet_type_taux);

	g_free (adr_common_str);
	g_free (adr_secondary_str);
	g_free (transactions_view);
	g_free (scheduler_column_width_write);
	g_free (transaction_column_width_write);
	g_free (transaction_column_align_write);

	g_free (date_format);
	g_free (mon_decimal_point);
	g_free (mon_thousands_sep);
	g_free (navigation_order_list);

	g_free (string_to_free1);
	g_free (string_to_free2);
	g_free (string_to_free3);

	g_free (form_organization);
	g_free (form_columns_width);

	g_free (bet_array_column_width_write);

	/* append the new string to the file content and return the new iterator */
	return gsb_file_save_append_part (iterator,
									  length_calculated,
									  file_content,
									  new_string);
}

/**
 * save the import rules structures
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 **/
static gulong gsb_file_save_import_rule_part (gulong iterator,
											  gulong *length_calculated,
											  gchar **file_content)
{
    GSList *list_tmp;

    list_tmp = gsb_data_import_rule_get_list ();

    while (list_tmp)
    {
		gchar *new_string;
		gchar *tmp_str;
		gint import_rule_number;
		const gchar *import_rule_type;

		import_rule_number = gsb_data_import_rule_get_number (list_tmp->data);
		import_rule_type = gsb_data_import_rule_get_type (import_rule_number);

		tmp_str = g_markup_printf_escaped ("\t<Import_rule Nb=\"%d\" Na=\"%s\" Acc=\"%d\" "
										   "Cur=\"%d\" Inv=\"%d\" Enc=\"%s\" Fil=\"%s\" Act=\"%d\" Typ=\"%s\" ",
										   import_rule_number,
										   my_safe_null_str(gsb_data_import_rule_get_name (import_rule_number)),
										   gsb_data_import_rule_get_account (import_rule_number),
										   gsb_data_import_rule_get_currency (import_rule_number),
										   gsb_data_import_rule_get_invert (import_rule_number),
										   my_safe_null_str(gsb_data_import_rule_get_charmap (import_rule_number)),
										   my_safe_null_str(gsb_data_import_rule_get_last_file_name
															(import_rule_number)),
										   gsb_data_import_rule_get_action (import_rule_number),
										   my_safe_null_str(import_rule_type));

		if (import_rule_type && strcmp (import_rule_type, "CSV") == 0)
		{
			gchar *tmp_str2;
			gint nbre_spec_lines;

			nbre_spec_lines = g_slist_length (gsb_data_import_rule_get_csv_spec_lines_list (import_rule_number));

			tmp_str2 = g_markup_printf_escaped ("IdC=\"%d\" IdR=\"%d\" FiS=\"%s\" Fld=\"%d\" Hp=\"%d\" Sep=\"%s\" "
												"SpCN=\"%s\" NbSL=\"%d\" />\n",
												gsb_data_import_rule_get_csv_account_id_col (import_rule_number),
												gsb_data_import_rule_get_csv_account_id_row (import_rule_number),
												gsb_data_import_rule_get_csv_fields_str (import_rule_number),
											    gsb_data_import_rule_get_csv_first_line_data (import_rule_number),
												gsb_data_import_rule_get_csv_headers_present (import_rule_number),
												gsb_data_import_rule_get_csv_separator (import_rule_number),
												gsb_data_import_rule_get_csv_spec_cols_name (import_rule_number),
												nbre_spec_lines
												);

			new_string = g_strconcat (tmp_str, tmp_str2, NULL);
			g_free (tmp_str);
			g_free (tmp_str2);

			if (nbre_spec_lines)
			{
				GSList *tmp_list;
				gint index = 1;

				/* append the new string to the file content and take the new iterator */
				iterator = gsb_file_save_append_part (iterator,
													  length_calculated,
													  file_content,
													  new_string);

				tmp_list = gsb_data_import_rule_get_csv_spec_lines_list (import_rule_number);
				while (tmp_list)
				{
					SpecConfData *spec_conf_data;

					spec_conf_data = (SpecConfData *) tmp_list->data;
					new_string = g_markup_printf_escaped ("\t<Special_line Nb=\"%d\" NuR=\"%d\" SpA=\"%d\" "
														  "SpAD=\"%d\" SpUD=\"%d\" SpUT=\"%s\" />\n",
														  index,
														  import_rule_number,
														  spec_conf_data->csv_spec_conf_action,
														  spec_conf_data->csv_spec_conf_action_data,
														  spec_conf_data->csv_spec_conf_used_data,
														  spec_conf_data->csv_spec_conf_used_text);

					/* append the new string to the file content and take the new iterator */
					iterator = gsb_file_save_append_part (iterator,
														  length_calculated,
														  file_content,
														  new_string);
					index++;

					tmp_list = tmp_list->next;
				}
			}
		}
		else
		{
			new_string = g_strconcat (tmp_str, "/>\n", NULL);
			g_free (tmp_str);

			/* append the new string to the file content and take the new iterator */
			iterator = gsb_file_save_append_part (iterator,
												  length_calculated,
												  file_content,
												  new_string);
		}

		list_tmp = list_tmp->next;
    }

	/* and return the new iterator */
	return iterator;
}

/**
 * save the partial_balance structures
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 **/
static gulong gsb_file_save_partial_balance_part (gulong iterator,
												  gulong *length_calculated,
												  gchar **file_content)
{
    GSList *list_tmp;

    list_tmp = gsb_data_partial_balance_get_list ();

    while (list_tmp)
    {
		gchar *new_string;
		gint partial_balance_number;

		partial_balance_number = gsb_data_partial_balance_get_number (list_tmp->data);

		new_string = g_markup_printf_escaped ("\t<Partial_balance Nb=\"%d\" Na=\"%s\" "
											  "Acc=\"%s\" Kind=\"%d\" Currency=\"%d\" Colorise=\"%d\" />\n",
											  partial_balance_number,
											  my_safe_null_str(gsb_data_partial_balance_get_name
															   (partial_balance_number)),
											  my_safe_null_str(gsb_data_partial_balance_get_liste_cptes
															   (partial_balance_number)),
											  gsb_data_partial_balance_get_kind (partial_balance_number),
											  gsb_data_partial_balance_get_currency (partial_balance_number),
											  gsb_data_partial_balance_get_colorise (partial_balance_number));

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
    }

	/* and return the new iterator */
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
 **/
static gulong gsb_file_save_party_part (gulong iterator,
										gulong *length_calculated,
										gchar **file_content)
{
    GSList *list_tmp;

    list_tmp = gsb_data_payee_get_payees_list ();

    while (list_tmp)
    {
        gchar *new_string;
        gint payee_number;

        payee_number = gsb_data_payee_get_no_payee (list_tmp->data);
        /* now we can fill the file content */

        if (gsb_data_payee_get_name (payee_number, TRUE) == NULL)
        {
            list_tmp = list_tmp->next;
            continue;
        }

        new_string = g_markup_printf_escaped ("\t<Party Nb=\"%d\" Na=\"%s\" Txt=\"%s\" "
											  "Search=\"%s\" IgnCase=\"%d\" UseRegex=\"%d\" />\n",
											  payee_number,
											  my_safe_null_str(gsb_data_payee_get_name (payee_number, TRUE)),
											  my_safe_null_str(gsb_data_payee_get_description (payee_number)),
											  my_safe_null_str(gsb_data_payee_get_search_string (payee_number)),
											  gsb_data_payee_get_ignore_case (payee_number),
											  gsb_data_payee_get_use_regex (payee_number));

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
    }

	/* and return the new iterator */
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
 **/
static gulong gsb_file_save_payment_part (gulong iterator,
										  gulong *length_calculated,
										  gchar **file_content)
{
	GSList *list_tmp;

	list_tmp = gsb_data_payment_get_payments_list ();

	while (list_tmp)
	{
		gint payment_number;
		gchar *new_string;

		payment_number = gsb_data_payment_get_number (list_tmp->data);

		/* now we can fill the file content */
		new_string = g_markup_printf_escaped ("\t<Payment Number=\"%d\" Name=\"%s\" Sign=\"%d\" Show_entry=\"%d\" "
											  "Automatic_number=\"%d\" Current_number=\"%s\" Account=\"%d\" />\n",
											  payment_number,
											  my_safe_null_str(gsb_data_payment_get_name (payment_number)),
											  gsb_data_payment_get_sign (payment_number),
											  gsb_data_payment_get_show_entry (payment_number),
											  gsb_data_payment_get_automatic_numbering (payment_number),
											  gsb_data_payment_get_last_number (payment_number),
											  gsb_data_payment_get_account_number (payment_number));

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
	}

	/* and return the new iterator */
	return iterator;
}

/**
 * save the print part
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 * \param archive_number the number of the archive or 0 if not an archive
 *
 * \return the new iterator
 **/
static gulong gsb_file_save_print_part (gulong iterator,
										gulong *length_calculated,
										gchar **file_content,
										gint archive_number)
{
	gchar *new_string;
	gchar *string_to_free1;
	gchar *string_to_free2;
	gchar *string_to_free3;
	gchar *string_to_free4;

	/* save the print config information */
	string_to_free1 = pango_font_description_to_string (gsb_data_print_config_get_font_transactions ());
	string_to_free2 = pango_font_description_to_string (gsb_data_print_config_get_font_title ());
	string_to_free3 = pango_font_description_to_string (gsb_data_print_config_get_report_font_transactions ());
	string_to_free4 = pango_font_description_to_string (gsb_data_print_config_get_report_font_title ());
	new_string = g_markup_printf_escaped ("\t<Print\n"
										  "\t\tDraw_lines=\"%d\"\n"
										  "\t\tDraw_column=\"%d\"\n"
										  "\t\tDraw_background=\"%d\"\n"
										  "\t\tDraw_archives=\"%d\"\n"
										  "\t\tDraw_columns_name=\"%d\"\n"
										  "\t\tDraw_title=\"%d\"\n"
										  "\t\tDraw_interval_dates=\"%d\"\n"
										  "\t\tDraw_dates_are_value_dates=\"%d\"\n"
										  "\t\tFont_transactions=\"%s\"\n"
										  "\t\tFont_title=\"%s\"\n"
										  "\t\tReport_font_transactions=\"%s\"\n"
										  "\t\tReport_font_title=\"%s\" />\n",
										  gsb_data_print_config_get_draw_lines (),
										  gsb_data_print_config_get_draw_column (),
										  gsb_data_print_config_get_draw_background (),
										  gsb_data_print_config_get_draw_archives (),
										  gsb_data_print_config_get_draw_columns_name (),
										  gsb_data_print_config_get_draw_title (),
										  gsb_data_print_config_get_draw_interval_dates (),
										  gsb_data_print_config_get_draw_dates_are_value_dates (),
										  my_safe_null_str(string_to_free1),
										  my_safe_null_str(string_to_free2),
										  my_safe_null_str(string_to_free3),
										  my_safe_null_str(string_to_free4));

		g_free (string_to_free1);
		g_free (string_to_free2);
		g_free (string_to_free3);
		g_free (string_to_free4);

	/* append the new string to the file content and return the new iterator */
	return gsb_file_save_append_part (iterator,
									  length_calculated,
									  file_content,
									  new_string);
}

/**
 * save the reconcile structures
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 **/
static gulong gsb_file_save_reconcile_part (gulong iterator,
											gulong *length_calculated,
											gchar **file_content)
{
    GList *list_tmp;

    list_tmp = gsb_data_reconcile_get_reconcile_list ();

    while (list_tmp)
    {
		gchar *new_string;
		gint reconcile_number;
		gchar *init_date;
		gchar *final_date;
		gchar *init_balance;
		gchar *final_balance;
		gint floating_point;

		reconcile_number = gsb_data_reconcile_get_no_reconcile (list_tmp->data);

		/* set the reconcile dates */
		init_date = gsb_format_gdate_safe (gsb_data_reconcile_get_init_date (reconcile_number));
		if (!init_date)
			init_date  = my_strdup ("");

		final_date = gsb_format_gdate_safe (gsb_data_reconcile_get_final_date (reconcile_number));
		if (!final_date)
			final_date = my_strdup ("");

		/* set the balances strings */
		floating_point = gsb_data_account_get_currency_floating_point (gsb_data_reconcile_get_account
																	   (reconcile_number));
		init_balance = gsb_real_safe_real_to_string (gsb_data_reconcile_get_init_balance
													 (reconcile_number),
													 floating_point);
		final_balance = gsb_real_safe_real_to_string (gsb_data_reconcile_get_final_balance
													  (reconcile_number),
													  floating_point);

		/* now we can fill the file content */
		new_string = g_markup_printf_escaped ("\t<Reconcile Nb=\"%d\" Na=\"%s\" Acc=\"%d\" "
											  "Idate=\"%s\" Fdate=\"%s\" Ibal=\"%s\" Fbal=\"%s\" />\n",
											  reconcile_number,
											  my_safe_null_str(gsb_data_reconcile_get_name (reconcile_number)),
											  gsb_data_reconcile_get_account (reconcile_number),
											  my_safe_null_str(init_date),
											  my_safe_null_str(final_date),
											  my_safe_null_str(init_balance),
											  my_safe_null_str(final_balance));

		g_free (init_date);
		g_free (final_date);
		g_free (init_balance);
		g_free (final_balance);

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
    }

	/* and return the new iterator */
	return iterator;
}

/**
 * save the rgba part
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 **/
static gulong gsb_file_save_rgba_part (gulong iterator,
									   gulong *length_calculated,
									   gchar **file_content)
{
	gchar *new_string;

	new_string = gsb_rgba_get_string_to_save ();

	/* append the new string to the file content and return the new iterator */
	return gsb_file_save_append_part (iterator,
									  length_calculated,
									  file_content,
									  new_string);
}

/**
 * save the scheduled transactions
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 **/
static gulong gsb_file_save_scheduled_part (gulong iterator,
											gulong *length_calculated,
											gchar **file_content)
{
	GSList *list_tmp;

	list_tmp = gsb_data_scheduled_get_scheduled_list ();

	while (list_tmp)
	{
		gint scheduled_number;
		gchar *new_string;
		gchar *amount;
		gchar *date;
		gchar *limit_date;
		gint floating_point;

		scheduled_number = gsb_data_scheduled_get_scheduled_number (list_tmp->data);

		/* set the real */
		floating_point = gsb_data_transaction_get_currency_floating_point (scheduled_number);
		amount = gsb_real_safe_real_to_string (gsb_data_scheduled_get_amount (scheduled_number),
											   floating_point);

		/* set the dates */
		date = gsb_format_gdate_safe (gsb_data_scheduled_get_date (scheduled_number));
		limit_date = gsb_format_gdate_safe (gsb_data_scheduled_get_limit_date (scheduled_number));

		/* now we can fill the file content */
		new_string = g_markup_printf_escaped ("\t<Scheduled Nb=\"%d\" Dt=\"%s\" Ac=\"%d\" Am=\"%s\" "
											  "Cu=\"%d\" Pa=\"%d\" Ca=\"%d\" Sca=\"%d\" Tra=\"%d\" Pn=\"%d\" "
											  "CPn=\"%d\" Pc=\"%s\" Fi=\"%d\" Bu=\"%d\" Sbu=\"%d\" No=\"%s\" "
											  "Au=\"%d\" Fd=\"%d\" Pe=\"%d\" Pei=\"%d\" Pep=\"%d\" Dtl=\"%s\" Br=\"%d\" "
											  "Mo=\"%d\" />\n",
											  scheduled_number,
											  my_safe_null_str(date),
											  gsb_data_scheduled_get_account_number (scheduled_number),
											  my_safe_null_str(amount),
											  gsb_data_scheduled_get_currency_number (scheduled_number),
											  gsb_data_scheduled_get_party_number (scheduled_number),
											  gsb_data_scheduled_get_category_number (scheduled_number),
											  gsb_data_scheduled_get_sub_category_number (scheduled_number),
											  gsb_data_scheduled_get_account_number_transfer (scheduled_number),
											  gsb_data_scheduled_get_method_of_payment_number (scheduled_number),
											  gsb_data_scheduled_get_contra_method_of_payment_number (scheduled_number),
											  my_safe_null_str(gsb_data_scheduled_get_method_of_payment_content (scheduled_number)),
											  gsb_data_scheduled_get_financial_year_number (scheduled_number),
											  gsb_data_scheduled_get_budgetary_number (scheduled_number),
											  gsb_data_scheduled_get_sub_budgetary_number (scheduled_number),
											  my_safe_null_str(gsb_data_scheduled_get_notes (scheduled_number)),
											  gsb_data_scheduled_get_automatic_scheduled (scheduled_number),
											  gsb_data_scheduled_get_fixed_date (scheduled_number),
											  gsb_data_scheduled_get_frequency (scheduled_number),
											  gsb_data_scheduled_get_user_interval (scheduled_number),
											  gsb_data_scheduled_get_user_entry (scheduled_number),
											  my_safe_null_str(limit_date),
											  gsb_data_scheduled_get_split_of_scheduled (scheduled_number),
											  gsb_data_scheduled_get_mother_scheduled_number (scheduled_number));

		g_free (amount);
		g_free (date);
		g_free (limit_date);

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
	}

	/* and return the new iterator */
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
 **/
static gulong gsb_file_save_transaction_part (gulong iterator,
											  gulong *length_calculated,
											  gchar **file_content,
											  gint archive_number)
{
	GSList *list_tmp;

	list_tmp = gsb_data_transaction_get_complete_transactions_list ();

	while (list_tmp)
	{
		gint transaction_number;
		gchar *new_string;
		gchar *amount;
		gchar *exchange_rate;
		gchar *exchange_fees;
		gchar *date;
		gchar *value_date;
		gint transaction_archive_number;
		gint floating_point;
		gint floating_fees;

		transaction_number = gsb_data_transaction_get_transaction_number (list_tmp->data);

		/* get the archive number for below */
		transaction_archive_number = gsb_data_transaction_get_archive_number (transaction_number);

		if (archive_number)
		{
			/* we export an archive, so continue only if the transaction belongs to that archive */
			if (transaction_archive_number != archive_number)
			{
				/* the transaction will not be exported */
				list_tmp = list_tmp->next;
				continue;
			}
			/* the transaction belongs to the archive,
			 * we set its archive number to 0, to show it when we open an archive */
			transaction_archive_number = 0;
		}

		/* set the reals. On met en forme le résultat pour avoir une cohérence dans les montants
		 * enregistrés dans le fichier à valider */
		floating_point = gsb_data_transaction_get_currency_floating_point (transaction_number);
		amount = gsb_real_safe_real_to_string (gsb_data_transaction_get_amount (transaction_number),
											   floating_point);
		exchange_rate = gsb_real_safe_real_to_string (gsb_data_transaction_get_exchange_rate
													  (transaction_number),
													  -1);
		floating_fees = gsb_data_account_get_currency_floating_point (gsb_data_transaction_get_account_number
																	  (transaction_number));
		exchange_fees = gsb_real_safe_real_to_string (gsb_data_transaction_get_exchange_fees
													  (transaction_number),
													  floating_fees );

		/* set the dates */
		date = gsb_format_gdate_safe (gsb_data_transaction_get_date (transaction_number));
		value_date = gsb_format_gdate_safe (gsb_data_transaction_get_value_date (transaction_number));

		/* now we can fill the file content */
		new_string = g_markup_printf_escaped ("\t<Transaction Ac=\"%d\" Nb=\"%d\" Id=\"%s\" Dt=\"%s\" "
											  "Dv=\"%s\" Cu=\"%d\" Am=\"%s\" Exb=\"%d\" Exr=\"%s\" Exf=\"%s\" "
											  "Pa=\"%d\" Ca=\"%d\" Sca=\"%d\" Br=\"%d\" No=\"%s\" Pn=\"%d\" "
											  "Pc=\"%s\" Ma=\"%d\" Ar=\"%d\" Au=\"%d\" Re=\"%d\" Fi=\"%d\" "
											  "Bu=\"%d\" Sbu=\"%d\" Vo=\"%s\" Ba=\"%s\" Trt=\"%d\" Mo=\"%d\" />\n",
											  gsb_data_transaction_get_account_number (transaction_number),
											  transaction_number,
											  my_safe_null_str(gsb_data_transaction_get_transaction_id (transaction_number)),
											  my_safe_null_str(date),
											  my_safe_null_str(value_date),
											  gsb_data_transaction_get_currency_number (transaction_number),
											  my_safe_null_str(amount),
											  gsb_data_transaction_get_change_between (transaction_number),
											  my_safe_null_str(exchange_rate),
											  my_safe_null_str(exchange_fees),
											  gsb_data_transaction_get_party_number (transaction_number),
											  gsb_data_transaction_get_category_number (transaction_number),
											  gsb_data_transaction_get_sub_category_number (transaction_number),
											  gsb_data_transaction_get_split_of_transaction (transaction_number),
											  my_safe_null_str(gsb_data_transaction_get_notes
															   (transaction_number)),
											  gsb_data_transaction_get_method_of_payment_number (transaction_number),
											  my_safe_null_str(gsb_data_transaction_get_method_of_payment_content
															   (transaction_number)),
											  gsb_data_transaction_get_marked_transaction (transaction_number),
											  transaction_archive_number,
											  gsb_data_transaction_get_automatic_transaction (transaction_number),
											  gsb_data_transaction_get_reconcile_number (transaction_number),
											  gsb_data_transaction_get_financial_year_number (transaction_number),
											  gsb_data_transaction_get_budgetary_number (transaction_number),
											  gsb_data_transaction_get_sub_budgetary_number (transaction_number),
											  my_safe_null_str(gsb_data_transaction_get_voucher (transaction_number)),
											  my_safe_null_str(gsb_data_transaction_get_bank_references
															   (transaction_number)),
											  gsb_data_transaction_get_contra_transaction_number (transaction_number),
											  gsb_data_transaction_get_mother_transaction_number (transaction_number));

		g_free (amount);
		g_free (exchange_rate);
		g_free (exchange_fees);
		g_free (date);
		g_free (value_date);

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		list_tmp = list_tmp->next;
	}

	/* and return the new iterator */
	return iterator;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
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
 **/
gboolean gsb_file_save_save_file (const gchar *filename,
								  gboolean compress,
								  gint archive_number)
{
	gint do_chmod;
	gulong iterator;

	gulong length_calculated;
	gchar *file_content;

	gint general_part = 4048;
	gint account_part = 1300;
	gint transaction_part = 350;
	gint party_part = 256;
	gint category_part = 500;
	gint budgetary_part = 500;
	gint currency_part = 100;
	gint currency_link_part = 100;
	gint bank_part = 500;
	gint financial_year_part = 100;
	gint archive_part = 120;
	gint reconcile_part = 150;
	gint report_part = 2500;
	gint import_rule_part = 50;
	gint partial_balance_part = 50;
	gint logo_part = 65536;
	gint bet_part = 500;
	gint bet_graph_part = 100;
	gint rgba_part = 1000;
	struct stat buf;
	GrisbiWinEtat *w_etat;

	devel_debug (filename);
	w_etat = grisbi_win_get_w_etat ();

	if (g_file_test (filename, G_FILE_TEST_EXISTS))
	{
		/* the file exists, we need to get the chmod values because gtk will overwrite it */
		if (stat (filename, &buf) == -1)
			/* stat couldn't get the information, so do as a new file
			 * and we will set the good chmod */
			do_chmod = TRUE;
		else
			do_chmod = FALSE;
	}
	else
		/* the file doesn't exist, so we will set the only user chmod */
		do_chmod = TRUE;

	run.file_is_saving = TRUE;

	/* we begin to try to reserve enough memory to make the entire file
	 * if not enough, we will make it growth later
	 * the data below are about the memory to take for each part and for 1 of this part
	 * with that i think we will allocate enough memory in one time but not too much */

	length_calculated = general_part
	+ account_part * gsb_data_account_get_number_of_accounts ()
	+ transaction_part * g_slist_length (gsb_data_transaction_get_complete_transactions_list ())
	+ party_part * g_slist_length (gsb_data_payee_get_payees_list ())
	+ category_part * g_slist_length (gsb_data_category_get_categories_list ())
	+ budgetary_part * g_slist_length (gsb_data_budget_get_budgets_list ())
	+ currency_part * g_slist_length (gsb_data_currency_get_currency_list ())
	+ currency_link_part * g_slist_length (gsb_data_currency_link_get_currency_link_list ())
	+ bank_part * g_slist_length (gsb_data_bank_get_bank_list ())
	+ financial_year_part * g_slist_length (gsb_data_fyear_get_fyears_list ())
	+ archive_part * g_slist_length (gsb_data_archive_get_archives_list ())
	+ reconcile_part * g_list_length (gsb_data_reconcile_get_reconcile_list ())
	+ report_part * g_slist_length (gsb_data_report_get_report_list ())
	+ import_rule_part * g_slist_length (gsb_data_import_rule_get_list ())
	+ partial_balance_part * g_slist_length (gsb_data_partial_balance_get_list ())
	+ logo_part
	+ bet_part
	+ bet_graph_part
	+ rgba_part;

	iterator = 0;
	file_content = g_malloc0 (length_calculated * sizeof (gchar));

	/* begin the file whith xml markup */
	iterator = gsb_file_save_append_part (iterator,
					   &length_calculated,
					   &file_content,
					   my_strdup ("<?xml version=\"1.0\"?>\n<Grisbi>\n"));

	iterator = gsb_file_save_general_part (iterator,
						&length_calculated,
						&file_content,
						archive_number);

	iterator = gsb_file_save_rgba_part (iterator,
					  &length_calculated,
					  &file_content);

	iterator = gsb_file_save_print_part (iterator,
					  &length_calculated,
					  &file_content,
					  archive_number);

	 iterator = gsb_file_save_currency_part (iterator,
						 &length_calculated,
						 &file_content);

	iterator = gsb_file_save_account_part (iterator,
						&length_calculated,
						&file_content);

	iterator = gsb_file_save_payment_part (iterator,
						&length_calculated,
						&file_content);

	iterator = gsb_file_save_transaction_part (iterator,
						&length_calculated,
						&file_content,
						archive_number);

	/* if we export an archive, no scheduled transactions */
	if (!archive_number)
	iterator = gsb_file_save_scheduled_part (iterator,
						  &length_calculated,
						  &file_content);

	iterator = gsb_file_save_party_part (iterator,
					  &length_calculated,
					  &file_content);

	iterator = gsb_file_save_category_part (iterator,
						 &length_calculated,
						 &file_content);

	iterator = gsb_file_save_budgetary_part (iterator,
						  &length_calculated,
						  &file_content);

	iterator = gsb_file_save_currency_link_part (iterator,
						  &length_calculated,
						  &file_content);

	iterator = gsb_file_save_bank_part (iterator,
					 &length_calculated,
					 &file_content);

	iterator = gsb_file_save_financial_year_part (iterator,
						   &length_calculated,
						   &file_content);

	/* if we export an archive, no archive information */
	if (!archive_number)
	iterator = gsb_file_save_archive_part (iterator,
						&length_calculated,
						&file_content);

	iterator = gsb_file_save_reconcile_part (iterator,
						  &length_calculated,
						  &file_content);

	iterator = gsb_file_save_import_rule_part (iterator,
						&length_calculated,
						&file_content);

	iterator = gsb_file_save_partial_balance_part (iterator,
						&length_calculated,
						&file_content);

	iterator = gsb_file_save_bet_part (iterator,
                        &length_calculated,
                        &file_content);

#ifdef HAVE_GOFFICE
	iterator = gsb_file_save_bet_graph_part (iterator,
						&length_calculated,
						&file_content);
#endif /* HAVE_GOFFICE */

	iterator = gsb_file_save_report_part (iterator,
					   &length_calculated,
					   &file_content,
					   FALSE);

	/* finish the file */
	iterator = gsb_file_save_append_part (iterator,
					   &length_calculated,
					   &file_content,
					   my_strdup ("</Grisbi>"));

	/* crypt the file if asked */
	if (w_etat->crypt_file)
	{
#ifdef HAVE_SSL
		{
			iterator = gsb_file_util_crypt_file (filename, &file_content, TRUE, iterator);
			if (!iterator)
			{
				g_free (file_content);
				return FALSE;
			}
		}
#else
		{
			gchar *text = _("This build of Grisbi does not support encryption.\n"
							"Please recompile Grisbi with OpenSSL encryption enabled.");
			gchar *hint;

			g_free (file_content);
			hint = g_strdup_printf (_("Cannot save encrypted file '%s'"), filename);
			dialogue_error_hint (text, hint);
			g_free (hint);

			return FALSE;
		}
#endif
	}

	/* the file is in memory, we can save it */
	/* i didn't succeed to save a "proper" file with zlib without compression,
	 * it always append some extra characters, so use glib without compression, and
	 * zlib if compression */
	if (compress)
	{
		gzFile grisbi_file;

		grisbi_file = gzopen (filename, "wb9");

		if (!grisbi_file
		 ||
		 !gzwrite (grisbi_file, file_content, (unsigned)iterator))
		{
			gchar* tmpstr = g_strdup_printf (_("Cannot save file '%s': %s"),
				              filename,
                              g_strerror(errno));
            dialogue_error (tmpstr);
            g_free (tmpstr);
            g_free (file_content);
            if (grisbi_file)
                gzclose (grisbi_file);

            return (FALSE);
        }

        g_free (file_content);
        gzclose (grisbi_file);
    }
    else
    {
        GError *error = NULL;

        if (!g_file_set_contents (filename, file_content, iterator, &error))
        {
            gchar* tmpstr = g_strdup_printf (_("Cannot save file '%s': %s"),
                              filename,
                              error->message);
            dialogue_error (tmpstr);
            g_free (tmpstr);
            g_error_free (error);

			return (FALSE);
        }
        g_free (file_content);
    }

    /* if it's a new file, we set the permission */
    if (do_chmod)
    {
        /* it's a new file or stat couldn't find the permissions,
         * so set only user can see the file by default */
        (void)chmod (filename, S_IRUSR | S_IWUSR);
	}
    else
    {
        /* it's not a new file but gtk overwrite the permissions
         * so need to re-set the good permissions saved before */
#if defined (_MINGW)
        if (_chmod (filename, buf.st_mode) == -1)
        {
            /* we couldn't set the chmod, set the default permission */
            _chmod (filename, _S_IREAD | _S_IWRITE);
        }
#else
        if (chmod (filename, buf.st_mode) == -1)
        {
            /* we couldn't set the chmod, set the default permission */
            (void)chmod (filename, S_IRUSR | S_IWUSR);
        }
        /* restores uid and gid */
/*        chown (filename, buf.st_uid, buf.st_gid);
*/#endif /*_MINGW */
    }

    run.file_is_saving = FALSE;

    return (TRUE);
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
 **/
gulong gsb_file_save_append_part (gulong iterator,
								  gulong *length_calculated,
								  gchar **file_content,
								  gchar *new_string)
{
	if (!new_string)
		return iterator;

	/* we check first if we don't go throw the upper limit */

	while ((iterator + strlen (new_string)) >= *length_calculated)
	{
		/* we change the size by adding half of length_calculated */

		*length_calculated = 1.5 * *length_calculated;
		*file_content = g_realloc (*file_content, *length_calculated);

		notice_debug ("The length calculated for saving file should be bigger?");
	}

	memcpy (*file_content + iterator, new_string, strlen (new_string));
	iterator = iterator + strlen (new_string);
	g_free (new_string);

	return iterator;
}

/**
 * Protect the caller against NULL strings.
 *
 * It simply returns the passed string when it is not NULL,
 * or "(null)" when it is NULL.
 *
 * \param string the string to be returned
 *
 * \return the given string, or "(null)" if it is a NULL pointer.
 **/
const gchar *my_safe_null_str (const gchar *string)
{
	if (string && strlen (string))
		return string;
	else
		return "(null)";
}

/**
 * save the budgetaries
 *
 * \param iterator the current iterator
 * \param length_calculated a pointer to the variable lengh_calculated
 * \param file_content a pointer to the variable file_content
 *
 * \return the new iterator
 **/
gulong gsb_file_save_budgetary_part (gulong iterator,
									 gulong *length_calculated,
									 gchar **file_content)
{
    GSList *list_tmp;

    list_tmp = gsb_data_budget_get_budgets_list ();

    while (list_tmp)
    {
		gchar *new_string;
		gchar *tmp_str;
		gint budget_number;
		GSList *sub_list_tmp;

		budget_number = gsb_data_budget_get_no_budget (list_tmp->data);

		/* now we can fill the file content */
		tmp_str = gsb_data_budget_get_name (budget_number, 0, "(null)");
		new_string = g_markup_printf_escaped ("\t<Budgetary Nb=\"%d\" Na=\"%s\" Kd=\"%d\" />\n",
										  budget_number,
										  tmp_str,
										  gsb_data_budget_get_type (budget_number));
		g_free (tmp_str);

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
										      file_content,
											  new_string);

		/* save the sub-budgetaries */
		sub_list_tmp = gsb_data_budget_get_sub_budget_list (budget_number);

		while (sub_list_tmp)
		{
			gint sub_budget_number;

			sub_budget_number = gsb_data_budget_get_no_sub_budget (sub_list_tmp->data);

			/* now we can fill the file content carrefull : the number of budget must be the first */
			tmp_str = gsb_data_budget_get_sub_budget_name (budget_number, sub_budget_number, "(null)");
			new_string = g_markup_printf_escaped ("\t<Sub_budgetary Nbb=\"%d\" Nb=\"%d\" Na=\"%s\" />\n",
												  budget_number,
												  sub_budget_number,
												  tmp_str);

			/* append the new string to the file content and take the new iterator */
			iterator = gsb_file_save_append_part (iterator,
												  length_calculated,
												  file_content,
												  new_string);

			sub_list_tmp = sub_list_tmp->next;
		}

		list_tmp = list_tmp->next;
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
 **/
gulong gsb_file_save_category_part (gulong iterator,
									gulong *length_calculated,
									gchar **file_content)
{
    GSList *list_tmp;

    list_tmp = gsb_data_category_get_categories_list ();

    while (list_tmp)
    {
		gchar *new_string;
		gchar *tmp_str;
		gint category_number;
		GSList *sub_list_tmp;

		category_number = gsb_data_category_get_no_category (list_tmp->data);

		/* now we can fill the file content */
		tmp_str = gsb_data_category_get_name (category_number, 0, "(null)");
		new_string = g_markup_printf_escaped ("\t<Category Nb=\"%d\" Na=\"%s\" Kd=\"%d\" />\n",
											  category_number,
											  tmp_str,
											  gsb_data_category_get_type (category_number));
		g_free (tmp_str);

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
										      file_content,
											  new_string);

		/* save the sub-categories */
		sub_list_tmp = gsb_data_category_get_sub_category_list (category_number);

		while (sub_list_tmp)
		{
			gint sub_category_number;

			sub_category_number = gsb_data_category_get_no_sub_category (sub_list_tmp->data);

			/* now we can fill the file content carrefull : the number of category must be the first */
			tmp_str = gsb_data_category_get_sub_category_name (category_number, sub_category_number, "(null)");
			new_string = g_markup_printf_escaped ("\t<Sub_category Nbc=\"%d\" Nb=\"%d\" Na=\"%s\" />\n",
												  category_number,
												  sub_category_number,
												  tmp_str);
			g_free (tmp_str);

			/* append the new string to the file content and take the new iterator */
			iterator = gsb_file_save_append_part (iterator,
												  length_calculated,
												  file_content,
												  new_string);

			sub_list_tmp = sub_list_tmp->next;
		}

		list_tmp = list_tmp->next;
    }

	return iterator;
}

/**
 * sauvegarde du fichier local en cours
 *
 * \param	données à sauvegarder dans un fichier local si modification des couleurs de base
 *
 * \return	FALSE si problème de sauvegarde sinon TRUE
 **/
gboolean gsb_file_save_css_local_file (const gchar *css_data)
{
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

	/* on sauvegarde le fichier CSS en local qu'en cas de modification */
	if (a_conf->prefs_change_css_data)
	{
		gchar *css_filename;
		gchar *tmp_str = NULL;
		GError *error = NULL;

		if (a_conf->use_type_theme == 2)
			tmp_str = g_strdup ("grisbi-dark.css");
		else
			tmp_str = g_strdup ("grisbi.css");

		css_filename = g_build_filename (gsb_dirs_get_user_config_dir (), tmp_str, NULL);
		g_free (tmp_str);

		if (!g_file_set_contents (css_filename, css_data, -1, &error))
		{
			tmp_str = g_strdup_printf (_("Cannot save CSS file '%s': %s"), css_filename, error->message);
            dialogue_error (tmp_str);
			g_free (tmp_str);
            g_error_free (error);

			return FALSE;
		}

		return TRUE;
	}

	return TRUE;
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
 **/
gulong gsb_file_save_report_part (gulong iterator,
								  gulong *length_calculated,
								  gchar **file_content,
								  gboolean current_report)
{
	GSList *list_tmp;

	list_tmp = gsb_data_report_get_report_list ();

	while (list_tmp)
	{
		gchar *new_string;
		gint report_number;
		gint report_number_to_write;
		GSList *tmp_list;
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

		report_number = gsb_data_report_get_report_number (list_tmp->data);

		/* if we need only the current report, we check here */
		if (current_report && gsb_gui_navigation_get_current_report() != report_number)
		{
			list_tmp = list_tmp->next;
			continue;
		}

		/* if report is a search report pas de sauvegarde */
		if (gsb_data_report_get_search_report (report_number))
		{
			list_tmp = list_tmp->next;
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
			report_name = g_strconcat (gsb_data_report_get_report_name (report_number),
									   " (export)",
									   NULL);
		else
			report_name = my_strdup (gsb_data_report_get_report_name (report_number));

		/* set the general sort type */
		tmp_list = gsb_data_report_get_sorting_type_list (report_number);
		general_sort_type = NULL;

		while (tmp_list)
		{
			if (general_sort_type)
			{
				gchar *string_to_free;
				gchar* tmp_str;

				tmp_str = general_sort_type;
				general_sort_type = g_strconcat (tmp_str,
												 "/-/",
												 string_to_free = utils_str_itoa (GPOINTER_TO_INT
																				  (tmp_list->data)),
												 NULL);
				g_free (tmp_str);
				g_free (string_to_free);
			}
			else
				general_sort_type = utils_str_itoa (GPOINTER_TO_INT (tmp_list->data));

			tmp_list = tmp_list->next;
		}

		/* set the financial_year_select */
		tmp_list = gsb_data_report_get_financial_year_list (report_number);
		financial_year_select = NULL;

		while (tmp_list)
		{
			if (financial_year_select)
			{
				gchar *string_to_free;
				gchar* tmp_str;

				tmp_str = financial_year_select ;
				financial_year_select = g_strconcat (tmp_str,
													 "/-/",
													 string_to_free = utils_str_itoa (GPOINTER_TO_INT
																					  (tmp_list->data)),
													 NULL);
				g_free (tmp_str);
				g_free (string_to_free);
			}
			else
				financial_year_select = utils_str_itoa (GPOINTER_TO_INT (tmp_list->data));

			tmp_list = tmp_list->next;
		}

		/* set the account_selected */
		tmp_list = gsb_data_report_get_account_numbers_list (report_number);
		account_selected = NULL;

		while (tmp_list)
		{
			if (account_selected)
			{
				gchar *string_to_free;
				gchar* tmp_str;

				tmp_str = account_selected;
				account_selected = g_strconcat (tmp_str,
												"/-/",
												string_to_free = utils_str_itoa (GPOINTER_TO_INT
																				 (tmp_list->data)),
												NULL);
				g_free (tmp_str);
				g_free (string_to_free);
			}
			else
				account_selected = utils_str_itoa (GPOINTER_TO_INT (tmp_list->data));

			tmp_list = tmp_list->next;
		}

		/* 	set the transfer_selected_accounts */
		tmp_list = gsb_data_report_get_transfer_account_numbers_list (report_number);
		transfer_selected_accounts = NULL;

		while (tmp_list)
		{
			if (transfer_selected_accounts)
			{
				gchar *string_to_free;
				gchar* tmp_str;

				tmp_str = transfer_selected_accounts;
				transfer_selected_accounts = g_strconcat (tmp_str,
														  "/-/",
														  string_to_free = utils_str_itoa (GPOINTER_TO_INT
																						   (tmp_list->data)),
														  NULL);
				g_free (tmp_str);
				g_free (string_to_free);
			}
			else
				transfer_selected_accounts = utils_str_itoa (GPOINTER_TO_INT (tmp_list->data));

			tmp_list = tmp_list->next;
		}

		/* save the category and sub-category list */
		tmp_list = gsb_data_report_get_category_struct_list (report_number);
		categ_selected = NULL;

		while (tmp_list)
		{
			CategBudgetSel *struct_categ = tmp_list->data;
			gchar *last_categ;
			gchar *new_categ;
			GSList *sub_categ_list;

			/* first, save the category */
			new_categ = utils_str_itoa (struct_categ->div_number);
			if (categ_selected)
			{
				last_categ = categ_selected;
				categ_selected = g_strconcat (last_categ, "-", new_categ, NULL);
				g_free (last_categ);
				g_free (new_categ);
			}
			else
				categ_selected = new_categ;

			/* if there are sub-categories, it's here */
			sub_categ_list = struct_categ->sub_div_numbers;
			while (sub_categ_list)
			{
				new_categ = utils_str_itoa (GPOINTER_TO_INT (sub_categ_list->data));

				last_categ = categ_selected;
				categ_selected = g_strconcat (last_categ, "/", new_categ, NULL);
				g_free (new_categ);
				g_free (last_categ);
				sub_categ_list = sub_categ_list->next;
			}
			tmp_list = tmp_list->next;
		}

		/* save the budget and sub-budget list */
		tmp_list = gsb_data_report_get_budget_struct_list (report_number);
		budget_selected = NULL;

		while (tmp_list)
		{
			CategBudgetSel *BudgetStruct = tmp_list->data;
			gchar *last_budget;
			gchar *new_budget;
			GSList *sub_budget_list;

			/* first, save the budget */
			new_budget = utils_str_itoa (BudgetStruct->div_number);
			if (budget_selected)
			{
				last_budget = budget_selected;

				budget_selected = g_strconcat (last_budget, "-", new_budget, NULL);
				g_free (last_budget);
				g_free (new_budget);
			}
			else
				budget_selected = new_budget;

			/* if there are sub-budgets, it's here */
			sub_budget_list = BudgetStruct->sub_div_numbers;
			while (sub_budget_list)
			{
				new_budget = utils_str_itoa (GPOINTER_TO_INT (sub_budget_list->data));

				last_budget = budget_selected;
				budget_selected = g_strconcat (last_budget, "/", new_budget, NULL);
				g_free (new_budget);
				g_free (last_budget);
				sub_budget_list = sub_budget_list->next;
			}
			tmp_list = tmp_list->next;
		}

		/* 	set the payee_selected */
		tmp_list = gsb_data_report_get_payee_numbers_list (report_number);
		payee_selected = NULL;

		while (tmp_list)
		{
			if (payee_selected)
			{
				gchar *string_to_free;
				gchar* tmp_str;

				tmp_str = payee_selected;
				payee_selected = g_strconcat (tmp_str,
											  "/-/",
											  string_to_free = utils_str_itoa (GPOINTER_TO_INT
																			   (tmp_list->data)),
											  NULL);
				g_free (tmp_str);
				g_free (string_to_free);
			}
			else
				payee_selected = utils_str_itoa (GPOINTER_TO_INT (tmp_list->data));

			tmp_list = tmp_list->next;
		}

		/* 	set the payment_method_list */
		tmp_list = gsb_data_report_get_method_of_payment_list (report_number);
		payment_method_list = NULL;

		while (tmp_list)
		{
			if (payment_method_list)
			{
				gchar* tmpstr = payment_method_list;

				payment_method_list = g_strconcat (tmpstr, "/-/", tmp_list->data, NULL);
				g_free (tmpstr);
			}
			else
				payment_method_list = my_strdup (tmp_list->data);

			tmp_list = tmp_list->next;
		}

		/* set the dates */
		date_start = gsb_format_gdate_safe (gsb_data_report_get_personal_date_start (report_number));
		date_end = gsb_format_gdate_safe (gsb_data_report_get_personal_date_end (report_number));

		/* now we can fill the file content */
		new_string = g_markup_printf_escaped ("\t<Report\n"
											  "\t\tNb=\"%d\"\n"
											  "\t\tName=\"%s\"\n"
											  "\t\tCompl_name_function=\"%d\"\n"
											  "\t\tCompl_name_position=\"%d\"\n"
											  "\t\tCompl_name_used=\"%d\"\n"
											  "\t\tGeneral_sort_type=\"%s\"\n"
											  "\t\tIgnore_archives=\"%d\"\n"
											  "\t\tShow_m=\"%d\"\n"
											  "\t\tShow_p=\"%d\"\n"
											  "\t\tShow_r=\"%d\"\n"
											  "\t\tShow_t=\"%d\"\n"
											  "\t\tShow_transaction=\"%d\"\n"
											  "\t\tShow_transaction_amount=\"%d\"\n"
											  "\t\tShow_transaction_nb=\"%d\"\n"
											  "\t\tShow_transaction_date=\"%d\"\n"
											  "\t\tShow_transaction_value_date=\"%d\"\n"
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
											  "\t\tShow_exclude_split_child=\"%d\"\n"
											  "\t\tShow_split_amounts=\"%d\"\n"
											  "\t\tCurrency_general=\"%d\"\n"
											  "\t\tReport_in_payees=\"%d\"\n"
											  "\t\tReport_can_click=\"%d\"\n"
											  "\t\tFinancial_year_used=\"%d\"\n"
											  "\t\tFinancial_year_kind=\"%d\"\n"
											  "\t\tFinancial_year_select=\"%s\"\n"
											  "\t\tDate_kind=\"%d\"\n"
											  "\t\tDate_select_value=\"%d\"\n"
											  "\t\tDate_beginning=\"%s\"\n"
											  "\t\tDate_end=\"%s\"\n"
											  "\t\tSplit_by_date=\"%d\"\n"
											  "\t\tSplit_date_period=\"%d\"\n"
											  "\t\tSplit_by_fin_year=\"%d\"\n"
											  "\t\tSplit_day_beginning=\"%d\"\n"
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
											  "\t\tCateg_show_amount=\"%d\"\n"
											  "\t\tCateg_show_sub_categ=\"%d\"\n"
											  "\t\tCateg_show_without_sub_categ=\"%d\"\n"
											  "\t\tCateg_show_sub_categ_amount=\"%d\"\n"
											  "\t\tCateg_currency=\"%d\"\n"
											  "\t\tCateg_show_name=\"%d\"\n"
											  "\t\tBudget_use=\"%d\"\n"
											  "\t\tBudget_use_selection=\"%d\"\n"
											  "\t\tBudget_selected=\"%s\"\n"
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
											  my_safe_null_str(report_name),
											  gsb_data_report_get_compl_name_function (report_number),
											  gsb_data_report_get_compl_name_position (report_number),
											  gsb_data_report_get_compl_name_used (report_number),
											  my_safe_null_str(general_sort_type),
											  gsb_data_report_get_ignore_archives (report_number),
											  gsb_data_report_get_show_m (report_number),
											  gsb_data_report_get_show_p (report_number),
											  gsb_data_report_get_show_r (report_number),
											  gsb_data_report_get_show_t (report_number),
											  gsb_data_report_get_show_report_transactions (report_number),
											  gsb_data_report_get_show_report_transaction_amount (report_number),
											  gsb_data_report_get_show_report_transaction_number (report_number),
											  gsb_data_report_get_show_report_date (report_number),
											  gsb_data_report_get_show_report_value_date (report_number),
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
											  gsb_data_report_get_not_detail_split (report_number),
											  gsb_data_report_get_split_credit_debit (report_number),
											  gsb_data_report_get_currency_general (report_number),
											  gsb_data_report_get_append_in_payee (report_number),
											  gsb_data_report_get_report_can_click (report_number),
											  gsb_data_report_get_use_financial_year (report_number),
											  gsb_data_report_get_financial_year_type (report_number),
											  my_safe_null_str(financial_year_select),
											  gsb_data_report_get_date_type (report_number),
											  gsb_data_report_get_date_select_value (report_number),
											  my_safe_null_str(date_start),
											  my_safe_null_str(date_end),
											  gsb_data_report_get_period_split (report_number),
											  gsb_data_report_get_period_split_type (report_number),
											  gsb_data_report_get_financial_year_split (report_number),
											  gsb_data_report_get_period_split_day (report_number) - G_DATE_MONDAY,
											  gsb_data_report_get_account_use_chosen (report_number),
											  my_safe_null_str(account_selected),
											  gsb_data_report_get_account_group_reports (report_number),
											  gsb_data_report_get_account_show_amount (report_number),
											  gsb_data_report_get_account_show_name (report_number),
											  gsb_data_report_get_transfer_choice (report_number),
											  my_safe_null_str(transfer_selected_accounts),
											  gsb_data_report_get_transfer_reports_only (report_number),
											  gsb_data_report_get_category_used (report_number),
											  gsb_data_report_get_category_detail_used (report_number),
											  my_safe_null_str(categ_selected),
											  gsb_data_report_get_category_show_category_amount (report_number),
											  gsb_data_report_get_category_show_sub_category (report_number),
											  gsb_data_report_get_category_show_without_category (report_number),
											  gsb_data_report_get_category_show_sub_category_amount (report_number),
											  gsb_data_report_get_category_currency (report_number),
											  gsb_data_report_get_category_show_name (report_number),
											  gsb_data_report_get_budget_used (report_number),
											  gsb_data_report_get_budget_detail_used (report_number),
											  my_safe_null_str(budget_selected),
											  gsb_data_report_get_budget_show_budget_amount (report_number),
											  gsb_data_report_get_budget_show_sub_budget (report_number),
											  gsb_data_report_get_budget_show_without_budget (report_number),
											  gsb_data_report_get_budget_show_sub_budget_amount (report_number),
											  gsb_data_report_get_budget_currency (report_number),
											  gsb_data_report_get_budget_show_name (report_number),
											  gsb_data_report_get_payee_used (report_number),
											  gsb_data_report_get_payee_detail_used (report_number),
											  my_safe_null_str(payee_selected),
											  gsb_data_report_get_payee_show_payee_amount (report_number),
											  gsb_data_report_get_payee_currency (report_number),
											  gsb_data_report_get_payee_show_name (report_number),
											  gsb_data_report_get_amount_comparison_currency (report_number),
											  gsb_data_report_get_amount_comparison_only_report_non_null (report_number),
											  gsb_data_report_get_method_of_payment_used (report_number),
											  my_safe_null_str(payment_method_list),
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

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

		/* save the text comparison */
		list_tmp_2 = gsb_data_report_get_text_comparison_list (report_number);

		while (list_tmp_2)
		{
			gint text_comparison_number;

			text_comparison_number = GPOINTER_TO_INT (list_tmp_2->data);

			/* if current_report is set, this mean we export the report,
			 * we cannot save the number should be automaticly given while
			 * importing, so set the report number to -1 */
			if (current_report)
				text_comparison_number_to_write = -1;
			else
				text_comparison_number_to_write = text_comparison_number;

			/* now we can fill the file content */
			new_string = g_markup_printf_escaped ("\t<Text_comparison\n"
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
												  gsb_data_report_text_comparison_get_link_to_last_text_comparison
												  (text_comparison_number),
												  gsb_data_report_text_comparison_get_field
												  (text_comparison_number),
												  gsb_data_report_text_comparison_get_operator
												  (text_comparison_number),
												  my_safe_null_str(gsb_data_report_text_comparison_get_text
																   (text_comparison_number)),
												  gsb_data_report_text_comparison_get_use_text
												  (text_comparison_number),
												  gsb_data_report_text_comparison_get_first_comparison
												  (text_comparison_number),
												  gsb_data_report_text_comparison_get_link_first_to_second_part
												  (text_comparison_number),
												  gsb_data_report_text_comparison_get_second_comparison
												  (text_comparison_number),
												  gsb_data_report_text_comparison_get_first_amount
												  (text_comparison_number),
												  gsb_data_report_text_comparison_get_second_amount
												  (text_comparison_number));

		/* append the new string to the file content and take the new iterator */
		iterator = gsb_file_save_append_part (iterator,
											  length_calculated,
											  file_content,
											  new_string);

			list_tmp_2 = list_tmp_2->next;
		}

		/* save the amount comparison */
		list_tmp_2 = gsb_data_report_get_amount_comparison_list (report_number);

		while (list_tmp_2)
		{
			gint amount_comparison_number;
			gchar *first_amount;
			gchar *second_amount;
			gint floating_point;

			amount_comparison_number = GPOINTER_TO_INT (list_tmp_2->data);

			/* if current_report is set, this mean we export the report,
			 * we cannot save the number should be automaticly given while
			 * importing, so set the report number to -1 */
			if (current_report)
				amount_comparison_number_to_write = -1;
			else
				amount_comparison_number_to_write = amount_comparison_number;

			/* set the numbers */
			floating_point = gsb_data_currency_get_floating_point (gsb_data_report_get_currency_general
																   (report_number));
			first_amount = gsb_real_safe_real_to_string (gsb_data_report_amount_comparison_get_first_amount
														 (amount_comparison_number),
														 floating_point);
			second_amount = gsb_real_safe_real_to_string (gsb_data_report_amount_comparison_get_second_amount
														  (amount_comparison_number),
														  floating_point);

			/* now we can fill the file content */
			new_string = g_markup_printf_escaped ("\t<Amount_comparison\n"
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
												  gsb_data_report_amount_comparison_get_link_to_last_amount_comparison
												  (amount_comparison_number),
												  gsb_data_report_amount_comparison_get_first_comparison
												  (amount_comparison_number),
												  gsb_data_report_amount_comparison_get_link_first_to_second_part
												  (amount_comparison_number),
												  gsb_data_report_amount_comparison_get_second_comparison
												  (amount_comparison_number),
												  my_safe_null_str(first_amount),
												  my_safe_null_str(second_amount));

			g_free (first_amount);
			g_free (second_amount);

			/* append the new string to the file content and take the new iterator */
			iterator = gsb_file_save_append_part (iterator, length_calculated, file_content, new_string);

			list_tmp_2 = list_tmp_2->next;
		}

		list_tmp = list_tmp->next;
	}

	return iterator;
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
