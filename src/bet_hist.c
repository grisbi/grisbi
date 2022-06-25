/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2007 Dominique Parisot                                   */
/*          zionly@free.org                                                   */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                 */
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
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "bet_hist.h"
#include "bet_data.h"
#include "bet_graph.h"
#include "bet_tab.h"
#include "dialog.h"
#include "export_csv.h"
#include "grisbi_app.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_fyear.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_fyear.h"
#include "gsb_real.h"
#include "gsb_rgba.h"
#include "mouse.h"
#include "navigation.h"
#include "print_tree_view_list.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_files.h"
#include "utils_real.h"
#include "utils_str.h"
#include "utils_widgets.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/* blocage des signaux pour le tree_view pour les comptes de type GSB_TYPE_CASH */
static gboolean hist_block_signal = FALSE;

/* toolbar */
static GtkWidget *bet_hist_toolbar;

/* liste qui contient les transactions concernées */
static GHashTable *list_trans_hist = NULL;

/**
 * this is a tree model filter with 3 columns :
 * the name, the number and a boolean to show it or not
 * */
static GtkTreeModel *bet_fyear_model = NULL;

/**
 * this is a tree model filter from fyear_model_filter which
 * show only the financial years which must be showed
 * */
static GtkTreeModel *bet_fyear_model_filter = NULL;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Cette fonction compare le montant de la colonne montant retenu avec le montant
 * de la colonne moyenne
 *
 * \param
 * \param
 *
 * \ return O si egal -1 ou 1 si différent
 **/
static gboolean bet_hist_amount_differ_average (GtkTreeModel *model,
												GtkTreeIter *iter)
{
	gchar *str_average;
	gchar *str_amount;

	gtk_tree_model_get (GTK_TREE_MODEL (model),
						iter,
						SPP_HISTORICAL_AVERAGE_AMOUNT, &str_average,
						SPP_HISTORICAL_RETAINED_AMOUNT, &str_amount,
						-1);

	return gsb_real_cmp (utils_real_get_from_string (str_average), utils_real_get_from_string (str_amount));
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static GsbReal bet_hist_get_children_amount (GtkTreeModel *model,
											 GtkTreeIter *parent)
{
	GtkTreeIter fils_iter;
	gchar *str_amount;
	GsbReal amount = null_real;

	if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &fils_iter, parent))
	{
		do
		{
			gtk_tree_model_get (GTK_TREE_MODEL (model),
								&fils_iter,
								SPP_HISTORICAL_RETAINED_AMOUNT, &str_amount,
								-1);
			amount = gsb_real_add (amount, utils_real_get_from_string (str_amount));
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &fils_iter));
	}

	return amount;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean bet_hist_get_full_div (GtkTreeModel *model,
									   GtkTreeIter *parent)
{
	GtkTreeIter fils_iter;
	gint valeur;

	if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &fils_iter, parent))
	{
		do
		{
			gtk_tree_model_get (GTK_TREE_MODEL (model),
								&fils_iter,
								SPP_HISTORICAL_SELECT_COLUMN, &valeur,
								-1);

			if (valeur == FALSE)
			{
				return FALSE;
			}
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &fils_iter));
	}
	return TRUE;
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
static void bet_hist_row_collapse_all (GtkTreeView *tree_view,
									   GtkTreeIter *iter,
									   GtkTreeModel *model)
{
	if (tree_view == NULL)
		tree_view = g_object_get_data (G_OBJECT (grisbi_win_get_account_page ()), "bet_hist_treeview");

	gtk_tree_view_collapse_row (tree_view, gtk_tree_model_get_path (model, iter));
	gtk_tree_selection_select_iter (gtk_tree_view_get_selection (tree_view), iter);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean bet_hist_set_empty_sub_div (GtkTreeModel *model,
											GtkTreeIter *parent)
{
	GtkTreeIter fils_iter;

	if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &fils_iter, parent))
	{
		gchar *str_amount;
		gint account_nb;
		gint div_number;
		gint sub_div_nb;

		do
		{
			gtk_tree_model_get (GTK_TREE_MODEL (model),
								&fils_iter,
								SPP_HISTORICAL_AVERAGE_AMOUNT, &str_amount,
								SPP_HISTORICAL_ACCOUNT_NUMBER, &account_nb,
								SPP_HISTORICAL_DIV_NUMBER, &div_number,
								SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
								-1);
			bet_data_hist_div_remove (account_nb, div_number, sub_div_nb);
			gtk_tree_store_set (GTK_TREE_STORE (model),
								&fils_iter,
								SPP_HISTORICAL_SELECT_COLUMN, FALSE,
								SPP_HISTORICAL_RETAINED_COLUMN, "",
								SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
								SPP_HISTORICAL_EDITED_COLUMN, TRUE,
								-1);
			g_free (str_amount);
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &fils_iter));
	}

	return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean bet_hist_set_full_sub_div (GtkTreeModel *model,
										   GtkTreeIter *parent)
{
	GtkTreeView *tree_view;
	GtkTreeIter fils_iter;
	gint edited = 0;

	if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &fils_iter, parent))
	{
		gchar *str_amount;
		gchar *str_retained;
		gint account_nb;
		gint div_number;
		gint sub_div_nb;
		GsbReal retained;

		do
		{
			gtk_tree_model_get (GTK_TREE_MODEL (model),
								&fils_iter,
								SPP_HISTORICAL_AVERAGE_AMOUNT, &str_amount,
								SPP_HISTORICAL_ACCOUNT_NUMBER, &account_nb,
								SPP_HISTORICAL_DIV_NUMBER, &div_number,
								SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
								SPP_HISTORICAL_EDITED_COLUMN, &edited,
								-1);

			retained = utils_real_get_from_string (str_amount);
			bet_data_hist_div_add (account_nb, div_number, sub_div_nb);
			bet_data_set_div_amount (account_nb, div_number, sub_div_nb, utils_real_get_from_string (str_amount));
			str_retained = utils_real_get_string_with_currency (retained,
																gsb_data_account_get_currency (account_nb),
																TRUE);
			gtk_tree_store_set (GTK_TREE_STORE (model),
								&fils_iter,
								SPP_HISTORICAL_SELECT_COLUMN, TRUE,
								SPP_HISTORICAL_RETAINED_COLUMN, str_retained,
								SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
								-1);
			g_free (str_amount);
			g_free (str_retained);
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &fils_iter));
	}

	if (edited)
	{
		tree_view = g_object_get_data (G_OBJECT (grisbi_win_get_account_page ()), "bet_hist_treeview");
		gtk_tree_view_collapse_row (tree_view, gtk_tree_model_get_path (model, parent));
		gtk_tree_selection_select_iter (gtk_tree_view_get_selection (tree_view), parent);
	}

	return FALSE;
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
static gboolean bet_hist_div_toggle_clicked (GtkCellRendererToggle *renderer,
											 gchar *path_string,
											 GtkTreeModel *model)
{
	GtkTreeIter iter;

	/* devel_debug (path_string); */
	if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path_string))
	{
		gchar *str_amount;
		gchar *str_average;
		gint div_number;
		gint sub_div_nb;
		gint nbre_fils;
		gint account_number;
		gboolean valeur;
		gboolean col_edited;

		gtk_tree_model_get (GTK_TREE_MODEL (model),
							&iter,
							SPP_HISTORICAL_SELECT_COLUMN, &valeur,
							SPP_HISTORICAL_AVERAGE_COLUMN, &str_average,
							SPP_HISTORICAL_AVERAGE_AMOUNT, &str_amount,
							SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
							SPP_HISTORICAL_DIV_NUMBER, &div_number,
							SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
							SPP_HISTORICAL_EDITED_COLUMN, &col_edited,
							-1);

		if (col_edited == FALSE && valeur == FALSE)
			return FALSE;

		if (renderer)
			valeur = 1 - valeur;

		if (valeur == 1)
		{
			/* printf ("avant - account_number = %d, div = %d, sub_div = %d\n", account_number, div, sub_div); */
			bet_data_hist_div_add (account_number, div_number, sub_div_nb);
			bet_data_set_div_amount (account_number, div_number, sub_div_nb, utils_real_get_from_string (str_amount));
			gtk_tree_store_set (GTK_TREE_STORE (model),
								&iter,
								SPP_HISTORICAL_SELECT_COLUMN, 1,
								SPP_HISTORICAL_RETAINED_COLUMN, str_average,
								SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
								SPP_HISTORICAL_ACCOUNT_NUMBER, account_number,
								SPP_HISTORICAL_DIV_NUMBER, div_number,
								SPP_HISTORICAL_SUB_DIV_NUMBER, sub_div_nb,
								-1);
		}
		else
		{
			bet_data_hist_div_remove (account_number, div_number, sub_div_nb);
			gtk_tree_store_set (GTK_TREE_STORE (model),
								&iter,
								SPP_HISTORICAL_SELECT_COLUMN, FALSE,
								SPP_HISTORICAL_RETAINED_COLUMN, "",
								SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
								SPP_HISTORICAL_ACCOUNT_NUMBER, account_number,
								SPP_HISTORICAL_DIV_NUMBER, div_number,
								SPP_HISTORICAL_SUB_DIV_NUMBER, sub_div_nb,
								SPP_HISTORICAL_EDITED_COLUMN, TRUE,
								-1);
		}

		nbre_fils = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (model), &iter);
		if (nbre_fils > 0)
		{
			if (valeur == 1)
				bet_hist_set_full_sub_div (model, &iter);
			else
				bet_hist_set_empty_sub_div (model, &iter);
		}
		else
		{
			GtkTreeIter parent;
			GsbReal amount;

			if (gtk_tree_model_iter_parent (GTK_TREE_MODEL (model), &parent, &iter))
			{
				if (bet_hist_get_full_div (model, &parent))
				{
					amount = bet_hist_get_children_amount (model, &parent);
					g_free (str_amount);
					str_amount = utils_real_get_string (amount);
					g_free (str_average);
					str_average = utils_real_get_string_with_currency (amount,
								gsb_data_account_get_currency (account_number), TRUE);
					gtk_tree_store_set (GTK_TREE_STORE (model),
								&parent,
								SPP_HISTORICAL_SELECT_COLUMN, TRUE,
								SPP_HISTORICAL_RETAINED_COLUMN, str_average,
								SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
								-1);
					if (bet_hist_amount_differ_average (model, &parent))
						bet_data_set_div_amount (account_number, div_number, 0, amount);

					bet_hist_row_collapse_all (NULL, &parent, model);
				}
				else
				{
					bet_data_set_div_amount (account_number, div_number, 0, null_real);
					gtk_tree_store_set (GTK_TREE_STORE (model), &parent,
								SPP_HISTORICAL_SELECT_COLUMN, FALSE,
								SPP_HISTORICAL_RETAINED_COLUMN, "",
								SPP_HISTORICAL_EDITED_COLUMN, TRUE,
								-1);
				}
			}
		}

		gsb_data_account_set_bet_maj (account_number, BET_MAJ_ESTIMATE);
		bet_data_update_bet_module (account_number, GSB_HISTORICAL_PAGE);
	}

	gsb_file_set_modified (TRUE);

	return FALSE;
}

/**
 * add the average amount of the item
 *
 * \param menu item
 * \param row selected
 *
 * \return
 **/
static void bet_hist_add_average_amount (GtkWidget *menu_item,
										 GtkTreeSelection *tree_selection)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *path_string;

	if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (tree_selection), &model, &iter))
		return;

	path_string = gtk_tree_path_to_string (gtk_tree_model_get_path (model, &iter));
	if (path_string)
		bet_hist_div_toggle_clicked (NULL, path_string, model);
}

/**
 * add the amount of the last transaction with the same division and sub-division
 *
 * \param menu item
 * \param row selected
 *
 * \return
 **/
static void bet_hist_add_last_amount (GtkWidget *menu_item,
									  GtkTreeSelection *tree_selection)
{
	GtkTreeModel *model;
	GtkTreeIter parent;
	GtkTreeIter iter;
	gint account_number;
	gint div_number;
	gint sub_div_nb;
	gint currency_number;
	gchar *tmp_str;
	gchar *str_amount;
	GsbReal amount;

	if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (tree_selection), &model, &iter))
		return;

	gtk_tree_model_get (GTK_TREE_MODEL (model),
						&iter,
						SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
						SPP_HISTORICAL_DIV_NUMBER, &div_number,
						SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
						-1);

	amount = gsb_data_transaction_get_last_transaction_with_div_sub_div (account_number,
																		 div_number,
																		 sub_div_nb,
																		 gsb_data_account_get_bet_hist_data
																		 (account_number));

	currency_number = gsb_data_account_get_currency (account_number);
	str_amount = utils_real_get_string (amount);

	tmp_str = utils_real_get_string_with_currency (amount, currency_number, TRUE);
	/* printf ("div = %d sub_div_nb = %d tmp_str = %s\n", div_number, sub_div_nb, tmp_str); */
	if (bet_data_hist_div_search (account_number, div_number, sub_div_nb) == FALSE)
		bet_data_hist_div_add (account_number, div_number, sub_div_nb);

	bet_data_set_div_edited  (account_number, div_number, sub_div_nb, TRUE);
	bet_data_set_div_amount (account_number, div_number, sub_div_nb, amount);
	gtk_tree_store_set (GTK_TREE_STORE (model),
						&iter,
						SPP_HISTORICAL_SELECT_COLUMN, TRUE,
						SPP_HISTORICAL_RETAINED_COLUMN, tmp_str,
						SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
						SPP_HISTORICAL_ACCOUNT_NUMBER, account_number,
						SPP_HISTORICAL_DIV_NUMBER, div_number,
						SPP_HISTORICAL_SUB_DIV_NUMBER, sub_div_nb,
						-1);

	if (gtk_tree_model_iter_parent (GTK_TREE_MODEL (model), &parent, &iter))
		gtk_tree_store_set (GTK_TREE_STORE (model), &parent, SPP_HISTORICAL_EDITED_COLUMN, FALSE, -1);

	gsb_data_account_set_bet_maj (account_number, BET_MAJ_ESTIMATE);

	gsb_file_set_modified (TRUE);
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
static void bet_hist_populate_div_model (gpointer key,
										 gpointer value,
										 gpointer user_data)
{
	HistData *shd = (HistData*) value;
	BetRange *sbr = shd->sbr;
	GtkTreeView *tree_view = (GtkTreeView *) user_data;
	GtkTreeModel *model;
	GtkTreeIter parent;
	GHashTableIter iter;
	gpointer sub_key, sub_value;
	gchar *div_name = NULL;
	gchar *str_balance;
	gchar *str_balance_amount;
	gchar *str_average;
	gchar *str_amount;
	gchar *str_retained = NULL;
	gchar *str_current_fyear;
	gint div_number;
	gint account_nb;
	gint currency_number;
	gint nbre_sub_div = 0;
	gboolean sub_div_visible = FALSE;
	gboolean edited = TRUE;
	GsbReal period = { 12, 0 };
	GsbReal average;
	GsbReal retained;
	GsbReal amount;
	KindAccount kind;
	GrisbiWinEtat *w_etat;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

	div_number = shd->div_number;
	div_name = bet_data_get_div_name (div_number, 0, NULL);
	account_nb = shd->account_nb;
	kind = gsb_data_account_get_kind (account_nb);
	if (kind == GSB_TYPE_CASH && w_etat->bet_cash_account_option == 0)
		edited = FALSE;

	currency_number = gsb_data_account_get_currency (account_nb);

	model = gtk_tree_view_get_model (tree_view);

	str_balance_amount = utils_real_get_string (sbr->current_balance);
	str_balance = utils_real_get_string_with_currency (sbr->current_balance, currency_number, TRUE);
	average = gsb_real_div (sbr->current_balance, period);
	str_amount = utils_real_get_string (average);
	str_average = utils_real_get_string_with_currency (average, currency_number, TRUE);

	str_current_fyear = utils_real_get_string_with_currency (sbr->current_fyear, currency_number, TRUE);

	gtk_tree_store_append (GTK_TREE_STORE (model), &parent, NULL);
	gtk_tree_store_set (GTK_TREE_STORE (model),
						&parent,
						SPP_HISTORICAL_DESC_COLUMN, div_name,
						SPP_HISTORICAL_CURRENT_COLUMN, str_current_fyear,
						SPP_HISTORICAL_BALANCE_COLUMN, str_balance,
						SPP_HISTORICAL_BALANCE_AMOUNT, str_balance_amount,
						SPP_HISTORICAL_AVERAGE_COLUMN, str_average,
						SPP_HISTORICAL_AVERAGE_AMOUNT, str_amount,
						SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
						SPP_HISTORICAL_ACCOUNT_NUMBER, account_nb,
						SPP_HISTORICAL_DIV_NUMBER, div_number,
						SPP_HISTORICAL_SUB_DIV_NUMBER, 0,
						SPP_HISTORICAL_EDITED_COLUMN, edited,
						-1);

	if (bet_data_hist_div_search (account_nb, div_number, 0)
	 && (bet_data_hist_get_div_edited (account_nb, div_number, 0)
		 || g_hash_table_size (shd->sub_div_list) < 1))
	{
		retained = bet_data_hist_get_div_amount (account_nb, div_number, 0);
		g_free (str_amount);
		str_amount = utils_real_get_string (retained);
		str_retained = utils_real_get_string_with_currency (retained, currency_number, TRUE);
		gtk_tree_store_set (GTK_TREE_STORE (model),
							&parent,
							SPP_HISTORICAL_SELECT_COLUMN, TRUE,
							SPP_HISTORICAL_RETAINED_COLUMN, str_retained,
							SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
							-1);
		g_free (str_retained);
	}
/* printf ("division = %d div_name = %s\n", div_number, div_name);  */
	g_free (div_name);
	g_free (str_balance);
	g_free (str_balance_amount);
	g_free (str_average);
	g_free (str_amount);
	g_free (str_current_fyear);

	if ((nbre_sub_div = g_hash_table_size (shd->sub_div_list)) < 1)
		return;

	g_hash_table_iter_init (&iter, shd->sub_div_list);
	while (g_hash_table_iter_next (&iter, &sub_key, &sub_value))
	{
		HistData *sub_shd = (HistData*) sub_value;
		BetRange *sub_sbr = sub_shd->sbr;
		GtkTreeIter fils;
		gchar **tab_str = NULL;

		if (nbre_sub_div == 1 && sub_shd->div_number == 0)
			return;

		div_name = bet_data_get_div_name (div_number, sub_shd->div_number, NULL);
/*		 printf ("division = %d sub_div = %d div_name = %s\n", div_number, sub_shd->div_number, div_name);  */
		if (div_name && g_utf8_strrchr (div_name, -1, ':'))
		{
			tab_str = g_strsplit (div_name, ":", 2);
			if (g_strv_length (tab_str) > 1)
			{
				g_free (div_name);
				div_name = g_strdup (g_strstrip (tab_str[1]));
			}
			g_strfreev (tab_str);
		}

		str_balance_amount = utils_real_get_string (sub_sbr->current_balance);
		str_balance = utils_real_get_string_with_currency (sub_sbr->current_balance, currency_number, TRUE);
		average = gsb_real_div (sub_sbr->current_balance, period);
		str_amount = utils_real_get_string (average);
		str_average = utils_real_get_string_with_currency (average, currency_number, TRUE);
		str_current_fyear = utils_real_get_string_with_currency (sub_sbr->current_fyear, currency_number, TRUE);

		gtk_tree_store_append (GTK_TREE_STORE (model), &fils, &parent);
		gtk_tree_store_set (GTK_TREE_STORE (model),
							&fils,
							SPP_HISTORICAL_DESC_COLUMN, div_name,
							SPP_HISTORICAL_CURRENT_COLUMN, str_current_fyear,
							SPP_HISTORICAL_BALANCE_COLUMN, str_balance,
							SPP_HISTORICAL_BALANCE_AMOUNT, str_balance_amount,
							SPP_HISTORICAL_AVERAGE_COLUMN, str_average,
							SPP_HISTORICAL_AVERAGE_AMOUNT, str_amount,
							SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
							SPP_HISTORICAL_ACCOUNT_NUMBER, account_nb,
							SPP_HISTORICAL_DIV_NUMBER, div_number,
							SPP_HISTORICAL_SUB_DIV_NUMBER, sub_shd->div_number,
							SPP_HISTORICAL_EDITED_COLUMN, edited,
							-1);

		if (bet_data_hist_div_search (account_nb, div_number, sub_shd->div_number))
		{
			GtkTreePath *path;

			if (bet_data_hist_get_div_edited (account_nb, div_number, sub_shd->div_number))
			{
				/* printf ("account_nb = %d div_number = %d sub_shd->div = %d\n", account_nb, div_number, sub_shd->div); */
				retained = bet_data_hist_get_div_amount (account_nb, div_number, sub_shd->div_number);
				g_free (str_amount);
				str_amount = utils_real_get_string (retained);
				str_retained = utils_real_get_string_with_currency (retained, currency_number, TRUE);

				edited = FALSE;
				gtk_tree_store_set (GTK_TREE_STORE (model),
									&parent,
									SPP_HISTORICAL_EDITED_COLUMN, edited,
									-1);
			}
			else
			{
				bet_data_set_div_amount (account_nb, div_number, sub_shd->div_number, average);
				str_retained = g_strdup (str_average);
				edited = TRUE;
			}
			gtk_tree_store_set (GTK_TREE_STORE (model),
								&fils,
								SPP_HISTORICAL_SELECT_COLUMN, TRUE,
								SPP_HISTORICAL_RETAINED_COLUMN, str_retained,
								SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
								-1);
			sub_div_visible = TRUE;
			path = gtk_tree_model_get_path (model, &fils);
			gtk_tree_view_expand_to_path (tree_view, path);

			g_free (str_retained);
			gtk_tree_path_free (path);
		}
		g_free (div_name);
		g_free (str_balance);
		g_free (str_balance_amount);
		g_free (str_average);
		g_free (str_amount);
		g_free (str_current_fyear);
	}

	if (sub_div_visible && bet_hist_get_full_div (model, &parent))
	{
		amount = bet_hist_get_children_amount (model, &parent);
		str_amount = utils_real_get_string (amount);
		str_retained = utils_real_get_string_with_currency (amount, currency_number, TRUE);

		gtk_tree_store_set (GTK_TREE_STORE (model),
							&parent,
							SPP_HISTORICAL_SELECT_COLUMN, TRUE,
							SPP_HISTORICAL_RETAINED_COLUMN, str_retained,
							SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
							SPP_HISTORICAL_EDITED_COLUMN, edited,
							-1);
		g_free (str_amount);
		g_free (str_retained);

		if (edited)
		{
			GtkTreePath *path;

			path = gtk_tree_model_get_path (model, &parent);
			gtk_tree_view_collapse_row (tree_view, path);
			gtk_tree_selection_select_iter (gtk_tree_view_get_selection (tree_view), &parent);

			gtk_tree_path_free (path);
		}
	}
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean bet_hist_affiche_div (GHashTable  *list_div,
									  GtkWidget *tree_view)
{
	/* devel_debug (NULL); */
	g_hash_table_foreach (list_div, bet_hist_populate_div_model, tree_view);

	return FALSE;
}

/**
 * Pop up a menu with several actions to apply to hist list.
 *
 * \param
 *
 * \return
 **/
static void bet_hist_context_menu (GtkWidget *tree_view)
{
	GtkWidget *menu, *menu_item;
	GtkTreeModel *model;
	GtkTreeSelection *tree_selection;
	GtkTreeIter iter;

	tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (tree_selection), &model, &iter))
		return;

	menu = gtk_menu_new ();

	/* Add last amount menu */
	menu_item = utils_menu_item_new_from_resource_label ("gtk-add-16.png",
														 _("Assign the amount of the last operation"));
	g_signal_connect (G_OBJECT (menu_item),
					  "activate",
					  G_CALLBACK (bet_hist_add_last_amount),
					  tree_selection);

	if (gtk_tree_model_iter_n_children (model, &iter) > 0)
		gtk_widget_set_sensitive (menu_item, FALSE);
	else
		gtk_widget_set_sensitive (menu_item, TRUE);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	/* Separator */
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new ());
	gtk_widget_show (menu_item);


	/* Add average amount menu */
	menu_item = utils_menu_item_new_from_image_label ("gtk-copy-16.png", _("Copy the average amount"));
	g_signal_connect (G_OBJECT (menu_item),
					  "activate",
					  G_CALLBACK (bet_hist_add_average_amount),
					  tree_selection);

	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	/* Finish all. */
	gtk_widget_show_all (menu);

	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
}

/**
 * called when we press a button on the list
 *
 * \param tree_view
 * \param ev
 *
 * \return FALSE
 **/
static gboolean bet_hist_button_press (GtkWidget *tree_view,
									   GdkEventButton *ev)
{
	/* show the popup */
	if (ev->button == RIGHT_BUTTON)
		bet_hist_context_menu (tree_view);

	return FALSE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_hist_export_tab (GtkWidget *menu_item,
								 GtkTreeView *tree_view)
{
	GtkWidget *dialog;
	gint resultat;
	gchar *filename;
	gchar *tmp_last_directory;

	dialog = gtk_file_chooser_dialog_new (_("Export the hist data"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_FILE_CHOOSER_ACTION_SAVE,
										  "gtk-cancel", GTK_RESPONSE_CANCEL,
										  "gtk-save", GTK_RESPONSE_OK,
										  NULL);

	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "division.csv");
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), gsb_file_get_last_path ());
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);

	resultat = gtk_dialog_run (GTK_DIALOG (dialog));

	switch (resultat)
	{
		case GTK_RESPONSE_OK :
			filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
			tmp_last_directory = utils_files_selection_get_last_directory (GTK_FILE_CHOOSER (dialog), TRUE);
			gsb_file_update_last_path (tmp_last_directory);
			g_free (tmp_last_directory);
			gtk_widget_destroy (GTK_WIDGET (dialog));

			/* vérification que c'est possible est faite par la boite de dialogue */
			if (!gsb_csv_export_tree_view_list (filename, tree_view))
			{
				dialogue_error (_("Cannot save file."));

				return;
			}
			break;

		default :
			gtk_widget_destroy (GTK_WIDGET (dialog));

			return;
	}
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void bet_hist_print_tab (GtkWidget *menu_item,
								GtkTreeView *tree_view)
{
	GtkTreeSelection *selection;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), NULL, NULL))
	{
		GtkTreePath *path;

		path = gtk_tree_path_new_first ();
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_path_free (path);
	}

	print_tree_view_list (menu_item, tree_view);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static GtkWidget *bet_hist_create_toolbar (GtkWidget *parent,
										   GtkWidget *tree_view)
{
	GtkWidget *toolbar;
	GtkToolItem *item;

	toolbar = gtk_toolbar_new ();
	g_object_set_data (G_OBJECT (toolbar), "tree_view", tree_view);
	g_object_set_data (G_OBJECT (toolbar), "page", parent);

	/* print button */
	item = utils_buttons_tool_button_new_from_image_label ("gtk-print-24.png", _("Print"));
	gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Print the array"));
	g_signal_connect (G_OBJECT (item), "clicked", G_CALLBACK (bet_hist_print_tab), tree_view);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

	/* Export button */
	item = utils_buttons_tool_button_new_from_image_label ("gsb-export-24.png", _("Export"));
	gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Export the array"));
	g_signal_connect (G_OBJECT (item), "clicked", G_CALLBACK (bet_hist_export_tab), tree_view);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

#ifdef HAVE_GOFFICE
	/* sectors button */
	item = utils_buttons_tool_button_new_from_image_label ("gsb-graph-sectors-24.png", _("Data graph"));
	gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Display the pie graph"));
	g_signal_connect (G_OBJECT (item), "clicked", G_CALLBACK (bet_graph_sectors_graph_new), tree_view);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

	/* monthly button */
	item = bet_graph_button_menu_new (toolbar, "hist_graph", G_CALLBACK (bet_graph_montly_graph_new), tree_view);
	gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
#endif /* HAVE_GOFFICE */

	return (toolbar);

}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void bet_hist_div_cell_edited (GtkCellRendererText *cell,
									  const gchar *path_string,
									  const gchar *new_text,
									  GtkWidget *tree_view)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	/* devel_debug (new_text); */
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	if (gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path_string))
	{
		GtkTreeIter parent;
		gboolean valeur;
		gboolean edited;
		gint account_number;
		gint currency_number;
		gint div_number;
		gint sub_div_nb;
		gchar *tmp_str;
		gchar *str_amount;
		gboolean is_parent = FALSE;
		GsbReal number;

		gtk_tree_model_get (GTK_TREE_MODEL (model),
							&iter,
							SPP_HISTORICAL_SELECT_COLUMN, &valeur,
							SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
							SPP_HISTORICAL_DIV_NUMBER, &div_number,
							SPP_HISTORICAL_SUB_DIV_NUMBER, &sub_div_nb,
							SPP_HISTORICAL_EDITED_COLUMN, &edited,
							-1);

		if (edited == FALSE)
			return;

		is_parent = gtk_tree_model_iter_parent (GTK_TREE_MODEL (model), &parent, &iter);
		if (is_parent == FALSE)
		{
			bet_hist_set_empty_sub_div (model, &iter);
			bet_hist_row_collapse_all (GTK_TREE_VIEW (tree_view), &iter, model);
		}

		number = utils_real_get_from_string (new_text);
		currency_number = gsb_data_account_get_currency (account_number);
		tmp_str = utils_real_get_string_with_currency (number, currency_number, TRUE);
		str_amount = utils_real_get_string (number);

		if (bet_data_hist_div_search (account_number, div_number, sub_div_nb) == FALSE)
			bet_data_hist_div_add (account_number, div_number, sub_div_nb);

		bet_data_set_div_edited  (account_number, div_number, sub_div_nb, TRUE);
		bet_data_set_div_amount (account_number, div_number, sub_div_nb, number);
		gtk_tree_store_set (GTK_TREE_STORE (model),
							&iter,
							SPP_HISTORICAL_SELECT_COLUMN, TRUE,
							SPP_HISTORICAL_RETAINED_COLUMN, tmp_str,
							SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
							-1);
		g_free (str_amount);
		g_free (tmp_str);

		if (is_parent)
			gtk_tree_store_set (GTK_TREE_STORE (model), &parent, SPP_HISTORICAL_EDITED_COLUMN, FALSE, -1);

		if (is_parent && bet_hist_get_full_div (model, &parent))
		{
			number = bet_hist_get_children_amount (model, &parent);
			bet_data_set_div_amount (account_number, div_number, 0, number);
			str_amount = utils_real_get_string (number);
			tmp_str = utils_real_get_string_with_currency (number, currency_number, TRUE);
			gtk_tree_store_set (GTK_TREE_STORE (model),
						&parent,
						SPP_HISTORICAL_SELECT_COLUMN, TRUE,
						SPP_HISTORICAL_RETAINED_COLUMN, tmp_str,
						SPP_HISTORICAL_RETAINED_AMOUNT, str_amount,
						SPP_HISTORICAL_EDITED_COLUMN, FALSE,
						-1);
			g_free (str_amount);
			g_free (tmp_str);
		}

		gsb_data_account_set_bet_maj (account_number, BET_MAJ_ESTIMATE);

		gsb_file_set_modified (TRUE);
	}
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void bet_hist_div_cell_editing_started (GtkCellRenderer *cell,
											  GtkCellEditable *editable,
											  const gchar *path_string,
											  GtkWidget *tree_view)
{
	if (GTK_IS_ENTRY (editable))
		gtk_editable_delete_text (GTK_EDITABLE (editable), 0, -1);
}

/**
 * set the background colors of the list
 *
 * \param tree_view
 *
 * \return FALSE
 **/
static gboolean bet_hist_set_background_color (GtkWidget *tree_view)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeIter fils_iter;

	if (!tree_view)
		return FALSE;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		GtkTreePath *path;
		gint current_color = 0;

		do
		{
			gtk_tree_store_set (GTK_TREE_STORE (model),
								&iter,
								SPP_HISTORICAL_BACKGROUND_COLOR,
								gsb_rgba_get_couleur_with_indice ("couleur_fond", current_color),
								-1);
			current_color = !current_color;

			path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter);
			if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &fils_iter, &iter)
			 && gtk_tree_view_row_expanded (GTK_TREE_VIEW (tree_view), path))
			{
				do
				{
					gtk_tree_store_set (GTK_TREE_STORE (model),
										&fils_iter,
										SPP_HISTORICAL_BACKGROUND_COLOR,
										gsb_rgba_get_couleur_with_indice ("couleur_fond", current_color),
										-1);
					current_color = !current_color;
				}
				while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &fils_iter));
			}
			gtk_tree_path_free (path);
		}
		while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
	}

	return FALSE;
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void bet_hist_row_collapsed_event (GtkTreeView *tree_view,
										  GtkTreeIter *iter,
										  GtkTreePath *path,
										  GtkTreeModel *model)
{
	bet_hist_set_background_color (GTK_WIDGET (tree_view));
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void bet_hist_row_expanded_event (GtkTreeView *tree_view,
										 GtkTreeIter *iter,
										 GtkTreePath *path,
										 GtkTreeModel *model)
{
	gint account_nb;
	gint div_number;

	gtk_tree_model_get (GTK_TREE_MODEL (model), iter,
						SPP_HISTORICAL_ACCOUNT_NUMBER, &account_nb,
						SPP_HISTORICAL_DIV_NUMBER, &div_number,
						-1);

	if (bet_data_hist_get_div_edited (account_nb, div_number, 0) == TRUE)
	{
		gtk_tree_view_collapse_row (tree_view, path);
		gtk_tree_selection_select_iter (gtk_tree_view_get_selection (tree_view), iter);
	}
	bet_hist_set_background_color (GTK_WIDGET (tree_view));
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *bet_hist_get_data_tree_view (GtkWidget *container)
{
	GtkWidget *scrolled_window;
	GtkWidget *tree_view;
	GtkWidget *account_page;
	GtkTreeStore *tree_model;
	GtkTreeSelection *tree_selection;
	GtkCellRenderer *cell;
	GtkTreeViewColumn *column;
	gchar *title;

	/* devel_debug (NULL); */
	account_page = grisbi_win_get_account_page ();
	tree_view = gtk_tree_view_new ();
	gtk_widget_set_tooltip_text (tree_view, _("Amount retained: Enter a negative number for a debit\n"
											  "a positive number for a credit"));
	g_object_set_data (G_OBJECT (account_page), "hist_tree_view", tree_view);

	tree_model = gtk_tree_store_new (SPP_HISTORICAL_NUM_COLUMNS,
									 G_TYPE_BOOLEAN,	/* SPP_HISTORICAL_SELECT_COLUMN	 */
									 G_TYPE_STRING,		/* SPP_HISTORICAL_DESC_COLUMN		*/
									 G_TYPE_STRING,		/* SPP_HISTORICAL_CURRENT_COLUMN	*/
									 G_TYPE_STRING,		/* SPP_HISTORICAL_BALANCE_COLUMN	*/
									 G_TYPE_STRING,		/* SPP_HISTORICAL_BALANCE_AMOUNT	*/
									 G_TYPE_STRING,		/* SPP_HISTORICAL_AVERAGE_COLUMN	*/
									 G_TYPE_STRING,		/* SPP_HISTORICAL_AVERAGE_AMOUNT	*/
									 G_TYPE_STRING,		/* SPP_HISTORICAL_RETAINED_COLUMN	*/
									 G_TYPE_STRING,		/* SPP_HISTORICAL_RETAINED_AMOUNT	*/
									 G_TYPE_STRING,		/* SPP_HISTORICAL_BALANCE_COLOR		*/
									 G_TYPE_INT,		/* SPP_HISTORICAL_ACCOUNT_NUMBER	*/
									 G_TYPE_INT,		/* SPP_HISTORICAL_DIV_NUMBER		*/
									 G_TYPE_INT,		/* SPP_HISTORICAL_SUB_DIV_NUMBER	*/
									 G_TYPE_BOOLEAN,	/* SPP_HISTORICAL_EDITED_COLUMN 	*/
									 GDK_TYPE_RGBA);	/* SPP_HISTORICAL_BACKGROUND_COLOR	*/
	gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (tree_model));
	g_object_unref (G_OBJECT (tree_model));

	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (tree_model),
										  SPP_HISTORICAL_DESC_COLUMN,
										  GTK_SORT_ASCENDING);

	tree_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
	gtk_tree_selection_set_mode (tree_selection, GTK_SELECTION_SINGLE);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
									GTK_POLICY_NEVER,
									GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request (scrolled_window, -1, 250);
	gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
	gtk_box_pack_start (GTK_BOX (container), scrolled_window, TRUE, TRUE, 15);

	/* create columns */
	cell = gtk_cell_renderer_toggle_new ();
	g_object_set_data (G_OBJECT (account_page), "toggle_cell", cell);
	gtk_cell_renderer_toggle_set_radio (GTK_CELL_RENDERER_TOGGLE (cell), FALSE);
	g_object_set (cell, "xalign", 0.5, NULL);
	g_signal_connect (cell, "toggled", G_CALLBACK (bet_hist_div_toggle_clicked), tree_model);

	column = gtk_tree_view_column_new_with_attributes (_("Select"),
													   cell,
													   "active", SPP_HISTORICAL_SELECT_COLUMN,
													   "cell-background-rgba", SPP_HISTORICAL_BACKGROUND_COLOR,
													   NULL);
	gtk_tree_view_column_set_alignment (column, 0.5);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
	gtk_tree_view_column_set_resizable (column, TRUE);
	g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (SPP_HISTORICAL_SELECT_COLUMN));

	/* name of the div sous-div column*/
	title = g_strdup (_("Category"));
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (title,
													   cell,
													   "text", SPP_HISTORICAL_DESC_COLUMN,
													   "cell-background-rgba", SPP_HISTORICAL_BACKGROUND_COLOR,
													   NULL);
	g_object_set_data (G_OBJECT (account_page), "hist_column_source", column);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
	gtk_tree_view_column_set_resizable (column, TRUE);
	g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (SPP_HISTORICAL_DESC_COLUMN));

	g_free (title);

	/* amount column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Amount"),
													   cell,
													   "text", SPP_HISTORICAL_BALANCE_COLUMN,
													   "foreground", SPP_HISTORICAL_BALANCE_COLOR,
													   "cell-background-rgba", SPP_HISTORICAL_BACKGROUND_COLOR,
													   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
	g_object_set (G_OBJECT (GTK_CELL_RENDERER (cell)), "xalign", 1.0, NULL);
	gtk_tree_view_column_set_alignment (column, 1);
	gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), FALSE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
	gtk_tree_view_column_set_resizable (column, TRUE);
	g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (SPP_HISTORICAL_BALANCE_COLUMN));

	/* average column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Average"),
													   cell,
													   "text", SPP_HISTORICAL_AVERAGE_COLUMN,
													   "foreground", SPP_HISTORICAL_BALANCE_COLOR,
													   "cell-background-rgba", SPP_HISTORICAL_BACKGROUND_COLOR,
													   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
	g_object_set (G_OBJECT (GTK_CELL_RENDERER (cell)), "xalign", 1.0, NULL);
	gtk_tree_view_column_set_alignment (column, 1);
	gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), FALSE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
	gtk_tree_view_column_set_resizable (column, TRUE);
	g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (SPP_HISTORICAL_AVERAGE_COLUMN));

	/* current fyear column */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Current fyear"),
													   cell,
													   "text", SPP_HISTORICAL_CURRENT_COLUMN,
													   "foreground", SPP_HISTORICAL_BALANCE_COLOR,
													   "cell-background-rgba", SPP_HISTORICAL_BACKGROUND_COLOR,
													   NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
	g_object_set (G_OBJECT (GTK_CELL_RENDERER (cell)), "xalign", 1.0, NULL);
	gtk_tree_view_column_set_alignment (column, 1);
	gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), FALSE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
	gtk_tree_view_column_set_resizable (column, TRUE);
	g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (SPP_HISTORICAL_CURRENT_COLUMN));

	/* amount retained column */
	cell = gtk_cell_renderer_text_new ();
	g_object_set_data (G_OBJECT (account_page), "edited_cell", cell);
	gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (cell), MARGIN_BOX, 0);
	column = gtk_tree_view_column_new_with_attributes (_("Amount retained"),
													   cell,
													   "text", SPP_HISTORICAL_RETAINED_COLUMN,
													   "foreground", SPP_HISTORICAL_BALANCE_COLOR,
													   "editable", SPP_HISTORICAL_EDITED_COLUMN,
													   "cell-background-rgba", SPP_HISTORICAL_BACKGROUND_COLOR,
													NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), GTK_TREE_VIEW_COLUMN (column));
	g_object_set (G_OBJECT (GTK_CELL_RENDERER (cell)), "xalign", 1.0, NULL);
	gtk_tree_view_column_set_alignment (column, 1);
	gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), FALSE);
	gtk_tree_view_column_set_expand (GTK_TREE_VIEW_COLUMN (column), TRUE);
	gtk_tree_view_column_set_resizable (column, TRUE);
	g_object_set_data (G_OBJECT (column), "num_col_model", GINT_TO_POINTER (SPP_HISTORICAL_RETAINED_COLUMN));

	g_signal_connect (cell,
					  "editing-started",
					  G_CALLBACK (bet_hist_div_cell_editing_started),
					  tree_view);
	g_signal_connect (cell,
					  "edited",
					  G_CALLBACK (bet_hist_div_cell_edited),
					  tree_view);

	g_signal_connect (G_OBJECT (tree_view),
					  "row-collapsed",
					  G_CALLBACK (bet_hist_row_collapsed_event),
					  tree_model);

	g_signal_connect (G_OBJECT (tree_view),
					  "row-expanded",
					  G_CALLBACK (bet_hist_row_expanded_event),
					  tree_model);

	g_signal_connect (G_OBJECT (tree_view),
					  "button-press-event",
					  G_CALLBACK (bet_hist_button_press),
					  NULL);

	gtk_widget_show_all (scrolled_window);

	return tree_view;
}

/**
 * discrimine les opérations appartenant à l'exercice en cours
 *
 * \param date					date de l'opération est > date_min de recherche
 * \param start_current_fyear	date de début de l'exercice en cours
 * \param date_max				date max de recherche des données passées
 *
 * \return 0	opération <= à date_max et < start_current_fyear (n'appartient pas à l'exercice en cours)
 * \return 1	opération > start_current_fyear (appartient à l'exercice en cours)
 * \return 2	opération <= à date_max et > start_current_fyear (appartient à l'exercice en cours) *
 * \return -1	toutes les autres opérations (aucune à priori)
 **/
static gint bet_hist_get_type_transaction (const GDate *date,
										   GDate *start_current_fyear,
										   GDate *date_max)
{
	gint result = -1;

	if (g_date_compare (date, date_max) <= 0)
	{
		if (g_date_compare (date, start_current_fyear) >= 0)
			result = 2;
		else
			result = 0;
	}
	else
	{
		if (g_date_compare (date, start_current_fyear) >= 0)
			result = 1;
	}

	return result;
}

/**
 *
 *
 * \param
 *
 * \return FALSE;
 **/
static gboolean bet_hist_initializes_account_settings (gint account_number)
{
	GtkWidget *account_page;
	GtkWidget *button = NULL;
	GtkWidget *combo = NULL;
	GtkWidget *label;
	GtkTreeViewColumn *column;
	gchar *title;
	gint fyear_number;
	gint origin_data;
	gint use_data_number;
	gpointer pointeur;

	account_page = grisbi_win_get_account_page ();

	/* set data origin_data */
	origin_data = gsb_data_account_get_bet_hist_data (account_number);
	if (origin_data)
	{
		button = g_object_get_data (G_OBJECT (account_page), "bet_hist_button_2");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
		bet_data_set_div_ptr (1);
		title = g_strdup (_("Budgetary line"));
	}
	else
	{
		button = g_object_get_data (G_OBJECT (account_page), "bet_hist_button_1");
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
		bet_data_set_div_ptr (0);
		title = g_strdup (_("Category"));
	}

	column = g_object_get_data (G_OBJECT (account_page), "hist_column_source");
	gtk_tree_view_column_set_title (GTK_TREE_VIEW_COLUMN (column), title);
	g_free (title);

	/* set fyear */
	combo = g_object_get_data (G_OBJECT (account_page), "bet_hist_fyear_combo");
	fyear_number = gsb_data_account_get_bet_hist_fyear (account_number);

	pointeur = g_object_get_data (G_OBJECT (combo), "pointer");
	g_signal_handlers_block_by_func (G_OBJECT (combo),
									 G_CALLBACK (utils_widget_origin_fyear_clicked),
									 pointeur);

	gsb_fyear_select_iter_by_number (combo, bet_fyear_model, bet_fyear_model_filter, fyear_number);

	g_signal_handlers_unblock_by_func (G_OBJECT (combo),
									   G_CALLBACK (utils_widget_origin_fyear_clicked),
									   pointeur);

	/* set label_use_data if necessary */
	label = g_object_get_data (G_OBJECT (grisbi_win_get_account_page ()), "bet_hist_label_use_data");
	use_data_number = gsb_data_account_get_bet_hist_use_data_in_account (account_number);
	if (use_data_number)
	{
		gchar *tmp_str;

		if (use_data_number == 1)
			tmp_str = g_strdup (_("(These data are aggregated with those "
								  "of a deferred debit card account)"));
		else
			tmp_str = g_strdup_printf (_("(These data are aggregated with those "
										 "of %d deferred debit card accounts)"),
									   use_data_number);

		gtk_label_set_text (GTK_LABEL (label), tmp_str);
		gtk_widget_show (label);

		g_free (tmp_str);
	}
	else
		gtk_widget_hide (label);

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GArray *bet_hist_get_cards_account_array_for_aggregate (gint account_number)
{
	GArray *garray;
	GHashTable *transfert_list;
	GHashTableIter iter;
	gpointer key, value;
	TransfertData *std;

	garray = g_array_new (FALSE, FALSE, sizeof (TransfertData *));

	transfert_list = bet_data_transfert_get_list ();
	g_hash_table_iter_init (&iter, transfert_list);
	while (g_hash_table_iter_next (&iter, &key, &value))
	{
		std = (TransfertData *) value;

		if (gsb_data_account_get_bet_hist_use_data_in_account (std->card_account_number)
			&& account_number == std->main_account_number)
		{
			g_array_append_val (garray, std);
		}
	}
	std = g_array_index (garray, TransfertData *, 0);

	return garray;
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
static gboolean bet_hist_valid_card_data_to_aggregate (gint tmp_account_number,
													   gint transaction_number,
													   GArray *garray)
{
	gint nbre_items;
	gint i;

	nbre_items = garray->len;
	for (i = 0; i < nbre_items; i++)
	{
		gint div;
		gint sub_div;
		gint origin_data;
		TransfertData *std;

		std = g_array_index (garray, TransfertData *, i);
		if (tmp_account_number == std->card_account_number)
		{
			origin_data = gsb_data_account_get_bet_hist_data (std->card_account_number);
			if (!origin_data)
			{
				div = gsb_data_transaction_get_category_number (transaction_number);
				sub_div = gsb_data_transaction_get_sub_category_number (transaction_number);
				if (div == std->card_category_number && sub_div == std->card_sub_category_number)
					return FALSE;
				else
					return TRUE;
			}
			else
			{
				div = gsb_data_transaction_get_budgetary_number (transaction_number);
				sub_div = gsb_data_transaction_get_sub_budgetary_number (transaction_number);
				if (div == std->card_budgetary_number && sub_div == std->card_sub_budgetary_number)
					return FALSE;
				else
					return TRUE;
			}
		}
		if (tmp_account_number == std->main_account_number)
		{
			origin_data = gsb_data_account_get_bet_hist_data (std->main_account_number);
			if (!origin_data)
			{
				div = gsb_data_transaction_get_category_number (transaction_number);
				sub_div = gsb_data_transaction_get_sub_category_number (transaction_number);
				if (div == std->main_category_number && sub_div == std->main_sub_category_number)
					return FALSE;
				else
					return TRUE;
			}
			else
			{
				div = gsb_data_transaction_get_budgetary_number (transaction_number);
				sub_div = gsb_data_transaction_get_sub_budgetary_number (transaction_number);
				if (div == std->main_budgetary_number && sub_div == std->main_sub_budgetary_number)
					return FALSE;
				else
					return TRUE;
			}
		}
	}

	return FALSE;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * Create the hist page
 *
 * \param
 *
 * \return
 **/
GtkWidget *bet_hist_create_page (void)
{
	GtkWidget *page;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *label_title;
	GtkWidget *label_use_data;
	GtkWidget *tree_view;
	GtkWidget *account_page;

	devel_debug (NULL);
	page = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
	gtk_widget_set_name (page, "hist_page");

	account_page = grisbi_win_get_account_page ();

	/* frame pour la barre d'outils */
	frame = gtk_frame_new (NULL);
	gtk_box_pack_start (GTK_BOX (page), frame, FALSE, FALSE, 0);

	/* titre de la page */
	label_title = gtk_label_new ("bet_hist_title");
	gtk_widget_set_halign (label_title, GTK_ALIGN_CENTER);
	gtk_box_pack_start (GTK_BOX (page), label_title, FALSE, FALSE, 0);
	g_object_set_data (G_OBJECT (grisbi_win_get_account_page ()), "bet_hist_title", label_title);

	/* Choix des données sources */
	hbox = utils_widget_origin_data_new (account_page, SPP_ORIGIN_HISTORICAL);
	gtk_widget_set_halign (hbox, GTK_ALIGN_CENTER);
	gtk_box_pack_start (GTK_BOX (page), hbox, FALSE, FALSE, 5);
	g_object_set_data (G_OBJECT (account_page), "bet_hist_data", hbox);

	/* label pour la fusion des données des CB à débit différé */
	label_use_data = gtk_label_new ("bet_hist_label_use_data");
	gtk_widget_set_halign (label_use_data, GTK_ALIGN_CENTER);
	gtk_label_set_selectable (GTK_LABEL (label_use_data), TRUE);
	gtk_box_pack_start (GTK_BOX (page), label_use_data, FALSE, FALSE, 0);
	g_object_set_data (G_OBJECT (grisbi_win_get_account_page ()), "bet_hist_label_use_data", label_use_data);

	/* création de la liste des données */
	tree_view = bet_hist_get_data_tree_view (page);
	g_object_set_data (G_OBJECT (account_page), "bet_hist_treeview", tree_view);
	g_object_set_data (G_OBJECT (tree_view), "label_title", label_title);

	/* set the color of selected row */
	gtk_widget_set_name (tree_view, "colorized_tree_view");

	/* on y ajoute la barre d'outils */
	bet_hist_toolbar = bet_hist_create_toolbar (page, tree_view);
	gtk_container_add (GTK_CONTAINER (frame), bet_hist_toolbar);

	gtk_widget_show_all (page);

	return page;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void bet_hist_populate_data (gint account_number)
{
	GtkWidget *tree_view;
	GtkTreeModel *model;
	GtkTreePath *path = NULL;
	gint fyear_number;
	GArray *garray = NULL;
	GDate *date_jour;
	GDate *date_min;
	GDate *date_max;
	GDate *start_current_fyear;
	GSList* tmp_list;
	GHashTable *list_div;

	devel_debug_int (account_number);
	tree_view = g_object_get_data (G_OBJECT (grisbi_win_get_account_page ()), "bet_hist_treeview");
	if (GTK_IS_TREE_VIEW (tree_view) == FALSE)
		return;

	/* Initializes account settings */
	bet_hist_initializes_account_settings (account_number);

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	gtk_tree_store_clear (GTK_TREE_STORE (model));

	/* calculate date_jour, date_min and date_max */
	date_jour = gdate_today ();

	fyear_number = gsb_data_account_get_bet_hist_fyear (account_number);
	if (fyear_number == 0)
	{
		date_min = gdate_today ();
		g_date_subtract_years (date_min, 1);
		date_max = gdate_today ();
		g_date_subtract_days (date_max, 1);
	}
	else
	{
		date_min = gsb_date_copy (gsb_data_fyear_get_beginning_date (fyear_number));
		date_max = gsb_date_copy (gsb_data_fyear_get_end_date (fyear_number));
	}

	/* calculate the current_fyear */
	start_current_fyear = bet_hist_get_start_date_current_fyear ();

	list_div = g_hash_table_new_full (g_str_hash,
									  g_str_equal,
									  (GDestroyNotify) g_free,
									  (GDestroyNotify) bet_data_hist_struct_free);

	/* on initialise ici la liste des transactions pour les graphiques mensuels */
	if (list_trans_hist)
		g_hash_table_remove_all (list_trans_hist);

	list_trans_hist = g_hash_table_new_full (g_str_hash,
											 g_str_equal,
											 (GDestroyNotify) g_free,
											 (GDestroyNotify) bet_data_struct_transaction_current_fyear_free);

	/* on traite la fusion des données des comptes CB à débit différé */
	if (gsb_data_account_get_bet_hist_use_data_in_account (account_number)
		&& gsb_data_account_get_kind (account_number) == GSB_TYPE_BANK)
	{
		garray = bet_hist_get_cards_account_array_for_aggregate (account_number);
	}

	/* search transactions of the account  */
	tmp_list = gsb_data_transaction_get_complete_transactions_list ();
	while (tmp_list)
	{
		gint transaction_number;
		gint tmp_account_number;
		const GDate *date;
		gint type_de_transaction;
		TransactionCurrentFyear *tcf = NULL;

		transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
		tmp_list = tmp_list->next;

		tmp_account_number =  gsb_data_transaction_get_account_number (transaction_number);
		if (garray == NULL)
		{
		if (tmp_account_number != account_number)
			continue;
		}
		else
		{
			if (!bet_hist_valid_card_data_to_aggregate (tmp_account_number, transaction_number, garray))
				continue;
		}
		date = gsb_data_transaction_get_date (transaction_number);

		/* ignore transaction which are before date_min */
		if (g_date_compare (date, date_min) < 0)
			continue;

		/* ignore transaction which is after date_jour */
		if (g_date_compare (date, date_jour) > 0)
			continue;

		/* ignore splitted transactions */
		if (gsb_data_transaction_get_split_of_transaction (
		 transaction_number) == TRUE)
			continue;

		/* on détermine le type de transaction pour l'affichage */
		type_de_transaction = bet_hist_get_type_transaction (date, start_current_fyear, date_max);

		tcf = bet_data_struct_transaction_current_fyear_init ();
		tcf->transaction_number = transaction_number;
		tcf->date = gsb_date_copy (date);
		tcf->type_de_transaction = type_de_transaction;

		g_hash_table_insert (list_trans_hist, utils_str_itoa (transaction_number), tcf);
		bet_data_hist_div_populate (transaction_number, TRUE, list_div, type_de_transaction, tcf);
	}

	bet_hist_affiche_div (list_div, tree_view);

	g_hash_table_remove_all (list_div);
	g_hash_table_unref (list_div);
	g_date_free (date_jour);
	g_date_free (date_min);
	g_date_free (date_max);
	g_date_free (start_current_fyear);

	bet_hist_set_background_color (tree_view);
	path = gtk_tree_path_new_first ();
	bet_array_list_select_path (tree_view, path);
	gtk_tree_path_free (path);
	if (garray)
		g_array_free (garray, FALSE);
}

/**
 * create and fill the list store of the fyear
 * come here mean that fyear_model_filter is NULL
 *
 * \param
 *
 * \return TRUE ok, FALSE problem
 **/
gboolean bet_hist_fyear_create_combobox_store (void)
{
	gchar *titre;

	/* devel_debug (NULL); */
	/* the fyear list store, contains 3 columns :
	 * FYEAR_COL_NAME : the name of the fyear
	 * FYEAR_COL_NUMBER : the number of the fyear
	 * FYEAR_COL_VIEW : it the fyear should be showed */

	titre = g_strdup (_("12 months rolling"));
	bet_fyear_model = GTK_TREE_MODEL (gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN));
	bet_fyear_model_filter = gtk_tree_model_filter_new (bet_fyear_model, NULL);
	gtk_tree_model_filter_set_visible_column (GTK_TREE_MODEL_FILTER (bet_fyear_model_filter), FYEAR_COL_VIEW);
	gsb_fyear_update_fyear_list_new (bet_fyear_model, bet_fyear_model_filter, titre);
	g_free (titre);

	return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint bet_hist_get_fyear_from_combobox (GtkWidget *combo_box)
{
	gint fyear_number = 0;
	GtkTreeIter iter;

	/* devel_debug (NULL); */
	if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo_box), &iter))
		return 0;

	if (bet_fyear_model_filter)
	{
		gtk_tree_model_get (GTK_TREE_MODEL (bet_fyear_model_filter),
							&iter,
							FYEAR_COL_NUMBER, &fyear_number,
							-1);
	}

	return fyear_number;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void bet_hist_set_fyear_from_combobox (GtkWidget *combo_box,
									   gint fyear)
{
	gsb_fyear_select_iter_by_number (combo_box, bet_fyear_model, bet_fyear_model_filter, fyear);
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
void bet_hist_refresh_data (GtkTreeModel *tab_model,
							GDate *date_min,
							GDate *date_max)
{
	GtkWidget *tree_view;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeIter fils_iter;

	devel_debug (NULL);
	tree_view = g_object_get_data (G_OBJECT (grisbi_win_get_account_page ()), "bet_hist_treeview");
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
	{
		gboolean valeur;
		gint account_number;

		/* test du numero de compte */
		gtk_tree_model_get (GTK_TREE_MODEL (model),
							&iter,
							SPP_HISTORICAL_ACCOUNT_NUMBER, &account_number,
							-1);
		if (account_number != gsb_gui_navigation_get_current_account ())
			return;

		do
		{
			gtk_tree_model_get (GTK_TREE_MODEL (model),
								&iter,
								SPP_HISTORICAL_SELECT_COLUMN, &valeur,
								-1);
			if (valeur == 1)
			{
				bet_array_list_add_new_hist_line (tab_model, GTK_TREE_MODEL (model), &iter, date_min, date_max);
			}
			else if (gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &fils_iter, &iter))
			{
				do
				{
					gtk_tree_model_get (GTK_TREE_MODEL (model),
										&fils_iter,
										SPP_HISTORICAL_SELECT_COLUMN, &valeur,
										-1);

					if (valeur == 1)
					{
						bet_array_list_add_new_hist_line (tab_model,
														  GTK_TREE_MODEL (model),
														  &fils_iter,
														  date_min,
														  date_max);
					}
				}
				while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &fils_iter));
			}

		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void bet_hist_g_signal_block_tree_view (void)
{
	GtkWidget *account_page;
	GtkTreeView *tree_view;
	gpointer cell;

	account_page = grisbi_win_get_account_page ();
	hist_block_signal = TRUE;
	tree_view = g_object_get_data (G_OBJECT (account_page), "hist_tree_view");

	/* set unsensitive toggle_cell and edited_cell */
	cell = g_object_get_data (G_OBJECT (account_page), "toggle_cell");
	gtk_cell_renderer_set_sensitive (GTK_CELL_RENDERER (cell), FALSE);
	cell = g_object_get_data (G_OBJECT (account_page), "edited_cell");
	gtk_cell_renderer_set_sensitive (GTK_CELL_RENDERER (cell), FALSE);

	g_signal_handlers_block_by_func (tree_view, bet_hist_button_press, NULL);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void bet_hist_g_signal_unblock_tree_view (void)
{
	GtkWidget *account_page;
	GtkTreeView *tree_view;
	gpointer cell;

	if (hist_block_signal == FALSE)
		return;
	account_page = grisbi_win_get_account_page ();

	hist_block_signal = FALSE;
	tree_view = g_object_get_data (G_OBJECT (account_page), "hist_tree_view");

	/* set sensitive toggle_cell and edited_cell */
	cell = g_object_get_data (G_OBJECT (account_page), "toggle_cell");
	gtk_cell_renderer_set_sensitive (GTK_CELL_RENDERER (cell), TRUE);
	cell = g_object_get_data (G_OBJECT (account_page), "edited_cell");
	gtk_cell_renderer_set_sensitive (GTK_CELL_RENDERER (cell), TRUE);

	g_signal_handlers_unblock_by_func (tree_view, bet_hist_button_press, NULL);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void bet_hist_set_page_title (gint account_number)
{
	GtkWidget *widget;
	gchar *title;
	gchar *hist_srce_name;
	gint result;

	hist_srce_name = bet_hist_get_hist_source_name (account_number);
	result = gsb_data_account_get_bet_hist_fyear (account_number);
	if (result == 0)
		title = g_strdup_printf (_("Amounts by %s on 12 months rolling for the account: '%s'"),
								 hist_srce_name,
								 gsb_data_account_get_name (account_number));
	else
		title = g_strdup_printf (_("Amounts by %s in %s for the account: '%s'"),
								 hist_srce_name,
								 gsb_data_fyear_get_name (result),
								 gsb_data_account_get_name (account_number));

	widget = GTK_WIDGET (g_object_get_data (G_OBJECT (grisbi_win_get_account_page ()), "bet_hist_title"));
	gtk_label_set_label (GTK_LABEL (widget), title);

	g_free (hist_srce_name);
	g_free (title);
}

/**
 * retourne la date de début de l'exercice en cours ou la date de l'année en cours.
 *
 * \param
 *
 * \return date de début de la période courante
 **/
GDate *bet_hist_get_start_date_current_fyear (void)
{
	GDate *date = NULL;
	gint fyear_number = 0;

	date = gdate_today ();
	fyear_number = gsb_data_fyear_get_from_date (date);

	if (fyear_number <= 0)
	{
		g_date_set_month (date, 1);
		g_date_set_day (date, 1);

		return date;
	}
	else
	{
		g_date_free (date);
		date = gsb_data_fyear_get_beginning_date (fyear_number);

		return gsb_date_copy (date);
	}
}

/**
 * cache l'exercice courant et les exercices futurs
 *
 * \param
 *
 * \return
 **/
void bet_hist_fyear_hide_present_futures_fyears (void)
{
	GDate *date;
	GSList *tmp_list;

	date = gdate_today ();

	tmp_list = gsb_data_fyear_get_fyears_list ();
	while (tmp_list)
	{
		FyearStruct *fyear;

		fyear = tmp_list->data;

		/* check the fyear only if the dates are valid */
		if (fyear->beginning_date && fyear->end_date)
		{
			if (g_date_compare (date, fyear->beginning_date) >= 0
			 &&
			 g_date_compare (date, fyear->end_date) <= 0)
			{
				gsb_fyear_hide_iter_by_name (bet_fyear_model, fyear->fyear_name);
			}
			else if (g_date_compare (date, fyear->beginning_date) <= 0)
			{
				gsb_fyear_hide_iter_by_name (bet_fyear_model, fyear->fyear_name);
			}
		}
		tmp_list = tmp_list->next;
	}

	g_date_free (date);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void bet_hist_update_toolbar (gint toolbar_style)
{
	gtk_toolbar_set_style (GTK_TOOLBAR (bet_hist_toolbar), toolbar_style);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gchar *bet_hist_get_hist_source_name (gint account_number)
{
	gint result ;

	if (account_number == -1)
		return g_strdup (_("Categories"));

	result = gsb_data_account_get_bet_hist_data (account_number);
	if (result)
		return g_strdup (_("Budgetary lines"));
	else
		return g_strdup (_("Categories"));
}

/**
 *
 *
 * \param
 *
 * \return TRUE
 * */
GHashTable *bet_hist_get_list_trans_current_fyear (void)
{
	return list_trans_hist;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkTreeModel *bet_hist_get_bet_fyear_model_filter (void)
{
	return bet_fyear_model_filter;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *bet_hist_get_toolbar (void)
{
	return bet_hist_toolbar;
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
