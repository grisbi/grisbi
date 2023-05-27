/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2008-2021 Pierre Biava (grisbi@pierre.biava.name)     */
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
 * \file gsb_data_account.c
 * work with the account structure, no GUI here
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_data_account.h"
#include "bet_data.h"
#include "custom_list.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_data_currency.h"
#include "gsb_data_form.h"
#include "gsb_data_import_rule.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_payment.h"
#include "gsb_data_report.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_dirs.h"
#include "gsb_file.h"
#include "gsb_select_icon.h"
#include "gsb_transactions_list.h"
#include "navigation.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

typedef struct	_AccountStruct 	AccountStruct;	/* struct_account describe an account */

struct _AccountStruct {
    /** @name general stuff */
    gint			account_number;
    gchar *			account_id;					/* for ofx import, invisible for the user */
    KindAccount		account_kind;
    gchar *			account_name;
    gint			currency;
    gint			closed_account;				/* if 1 => closed */
    gchar *			comment;
    gchar *			holder_name;
    gchar *			holder_address;
	gboolean		has_pointed;				/* TRUE if one transaction is pointed */

    /** @name account_icon */
    gchar *			name_icon;					/* path for not standard icon */
    GdkPixbuf *		pixbuf;						/* pixbuf for the account */

    /** @name method of payment */
    gint			default_debit;
    gint			default_credit;

    /** @name showed list stuff */
    gint			show_r;                     /* 1 : reconciled transactions are showed */
    gint			nb_rows_by_transaction;     /* 1, 2, 3, 4  */
    gint			show_l;                     /** 1 archived lines are showed */

    /** @name remaining of the balances */
    gboolean		balances_are_dirty;
    GsbReal			current_balance;
    GsbReal			init_balance;
    GsbReal			marked_balance;
    GsbReal			mini_balance_wanted;
    GsbReal			mini_balance_authorized;

    /** @name remaining of the minimun balance message */
    gint			mini_balance_wanted_message;
    gint			mini_balance_authorized_message;

    /** @name number of the transaction selectionned, or -1 for the white line */
    gint			current_transaction_number;

    /** @name bank stuff */
    gint			bank_number;
    gchar *			bank_branch_code;
    gchar *			bank_account_number;
    gchar *			bank_account_key;
    gchar *			bank_account_iban;

    /** @name reconcile sort */
    gint			reconcile_sort_type;    /* 1 : sort by method of payment ; 0 : let the user sort by himself */
    GSList 	*		sort_list;				/* the method of payment numbers sorted in a list
							        			(if split neutral, the negative method has a negative method of payment number)*/
    gint			split_neutral_payment;	/* if 1 : neutral payments are splitted into debits/credits */

    /** @name tree_view sort stuff */
    gint			sort_type;				/* GTK_SORT_DESCENDING / GTK_SORT_ASCENDING */
    gint			sort_column;		    /* used to hide the arrow when change the column */
    gint			column_element_sort[CUSTOM_MODEL_VISIBLE_COLUMNS];  /* contains for each column the element number used to sort the list */

    /** @name current graphic position in the list (the row_align used with gtk_tree_view_scroll_to_cell) */
    gfloat			row_align;

    /** @name bet data */
    gint			bet_use_budget;					/* -1 = pas de module possible 0 = non utilisé 1 = utilisé */
    gint			bet_credit_card;				/* 1 = compte type CB à débit différé */
    BetTypeOnglets	bet_show_onglets;  				/* enum des onglets à afficher pour le module budgetaire */
    GDate *			bet_start_date;					/* date de début */
    gint			bet_months;						/* nombre de mois ou d'années */
    gint			bet_spin_range;					/* echelle de la période 0 = mois 1 = années */
    gint			bet_auto_inc_month;				/* incrémente automatiquement le mois */
    gint			bet_select_transaction_label;	/* fixe le label pour les opérations */
    gint			bet_select_scheduled_label;		/* fixe le label pour les opérations planifiées */
    gint			bet_select_futur_label;			/* fixe le label pour les données futures */
    gint			bet_hist_data;					/* origine des données 0 = catégories 1 = IB */
    gint			bet_hist_fyear;					/* numéro d'exercice */
	gint			bet_hist_use_data_in_account;	/* utilise les données historique du compte lié dans le compte principal */
													/* compte lié = 0 ou 1, compte maitre = nbre de comptes concernés */
    gint			bet_maj;						/* MAJ du module estiamte balance */
    gdouble			bet_capital;					/* capital emprunté */
    gdouble			bet_taux_annuel;				/* taux d'interet annuel */
    gdouble			bet_frais;						/* frais par echeance */
    gint			bet_type_taux;					/* type de taux : actuariel ou proportionnel */
	gboolean		bet_init_sch_with_loan;			/* Initialise l'opération planifiée avec le tableau d'amortissement */
	gboolean		bet_split_transaction;			/* crée une opération ventilée dans le compte principal */
};

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
/** contains a g_slist of struct_account in the good order */
static GSList *list_accounts = NULL;

/** a pointer to the last account used (to increase the speed) */
static AccountStruct *account_buffer;
/*END_STATIC*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * This internal function is called to free the memory used by an AccountStruct structure
 *
 * \param
 *
 * \return
 **/
static void _gsb_data_account_free (AccountStruct* account)
{
    if (!account)
        return;

    g_free (account->account_id);
    g_free (account->account_name);
    g_free (account->name_icon);
    g_free (account->comment);
    g_free (account->holder_name);
    g_free (account->holder_address);
    g_free (account->bank_branch_code);
    g_free (account->bank_account_number);
    g_free (account->bank_account_key);
    g_free (account->bank_account_iban);
    if (account->sort_list)
        g_slist_free(account->sort_list) ;
    if (account->bet_start_date)
        g_date_free (account->bet_start_date);
    if (G_IS_OBJECT (account->pixbuf))
        g_object_unref (account->pixbuf);
    if (account_buffer == account)
        account_buffer = NULL;
    g_free (account);
}

/**
 * get the filename of the default pixbuf icon for the kind_account
 *
 * \param kind_account
 *
 * \return filename that must be freed with g_free ()
 **/
static gchar *gsb_data_account_get_account_standard_pixbuf_filename (KindAccount account_kind)
{
    const gchar *account_icon;
    gchar *filename;

    switch (account_kind)
    {
		case GSB_TYPE_BANK:
			account_icon = "gsb-ac-bank-32.png";
			break;
		case GSB_TYPE_CASH:
			account_icon = "gsb-ac-cash-32.png";
			break;
		case GSB_TYPE_ASSET:
		account_icon = "gsb-ac-asset-32.png";
			break;
		case GSB_TYPE_LIABILITIES:
			account_icon = "gsb-ac-liability-32.png";
			break;
		case GSB_TYPE_BALANCE:
		default:
			account_icon = "gsb-ac-bank-32.png";
			break;
    }

    filename = g_build_filename (gsb_dirs_get_pixmaps_dir (), account_icon, NULL);

    return filename;
}

/**
 * find and return the structure of the account asked
 *
 * \param no number of account
 *
 * \return the adr of the struct of the account (NULL if doesn't exit)
 **/
static AccountStruct *gsb_data_account_get_structure (gint no)
{
    GSList *tmp;

    if (no <= 0)
    {
		return NULL;
    }

    /* before checking all the accounts, we check the buffer */
    if (account_buffer && account_buffer->account_number == no)
		return account_buffer;

    tmp = list_accounts;
    while (tmp)
    {
		AccountStruct *account;

		account = tmp->data;
		if (account && account->account_number == no)
		{
			account_buffer = account;
			return account;
		}

		tmp = tmp->next;
    }

	return NULL;
}

/**
 * find and return the last number of account
 *
 * \param none
 *
 * \return last number of account
 **/
static gint gsb_data_account_max_number (void)
{
    GSList *tmp;
    gint number_tmp = 0;

    tmp = list_accounts;
    while (tmp)
    {
		AccountStruct *account;

		account = tmp->data;
		if (!account)
			return 0;

		if (account->account_number > number_tmp)
			number_tmp = account->account_number;

		tmp = tmp->next;
    }

	return number_tmp;
}

/**
 * initalize the sort variables for an account to the default value
 * used normally only when creating a new account if it's the first one
 * in all others cases, we will take a copy of that values of the previous account
 *
 * \param account_number
 *
 * \return FALSE
 **/
static gboolean gsb_data_account_set_default_sort_values (gint account_number)
{
	gint *ptr;
    gint i;
    gint j;
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
	ptr = gsb_transactions_list_get_tab_affichage_ope ();

    if (!account)
		return FALSE;

    for (i = 0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
	for (j = 0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
	{
		gint element_number;

		element_number = *(ptr + (i * CUSTOM_MODEL_VISIBLE_COLUMNS) + j);

		/* by default the sorting element will be the first found for each column */
	    if (!account->column_element_sort[j] && element_number)
			account->column_element_sort[j] = element_number;
	}

    /* the default sort is by date and ascending */
    account->sort_type = GTK_SORT_ASCENDING;
    account->sort_column = CUSTOM_MODEL_COL_1;

	return FALSE;
}

/**
 * copy the sort values from an account to another
 *
 * \param origin_account
 * \param target_account
 *
 * \return TRUE ok, FALSE problem
 **/
static gboolean gsb_data_account_dup_sort_values (gint origin_account,
												  gint target_account)
{
    gint j;
    AccountStruct *origin_account_ptr;
    AccountStruct *target_account_ptr;

    origin_account_ptr = gsb_data_account_get_structure (origin_account);
    target_account_ptr = gsb_data_account_get_structure (target_account);

    if (!origin_account_ptr || !target_account_ptr)
		return FALSE;

    for (j = 0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
		target_account_ptr->column_element_sort[j] = origin_account_ptr->column_element_sort[j];

    target_account_ptr->sort_type   = origin_account_ptr->sort_type;
    target_account_ptr->sort_column = origin_account_ptr->sort_column;

	return TRUE;
}

/**
 * fonction de comparaison de 2 pointeurs de gint
 *
 * \param
 * \param
 *
 * \return
 **/
static gint gsb_data_account_cmp_numbers (gconstpointer a,
										  gconstpointer b)
{
	gint a_num;
	gint b_num;

	if (a == NULL)
		return -1;
	if (b == NULL)
		return 1;

	a_num = GPOINTER_TO_INT (a);
	b_num = GPOINTER_TO_INT (b);
	if (a_num < b_num)
		return -1;
	else if (a_num == b_num)
		return 0;
	else
		return 1;
}

/**
 * This function close all opened accounts and free the memory
 * used by them.
 *
 * \param
 *
 * \return
 **/
static void gsb_data_account_delete_all_accounts (void)
{
    if (list_accounts)
    {
        g_slist_free_full (list_accounts, (GDestroyNotify) _gsb_data_account_free);
        list_accounts = NULL;
    }

    account_buffer = NULL;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * set the accounts global variables to NULL,
 * usually when we init all the global variables
 *
 * \param none
 *
 * \return FALSE
 **/
gboolean gsb_data_account_init_variables (void)
{
    gsb_data_account_delete_all_accounts ();

    return FALSE;
}

/**
 * return a pointer on the g_slist of accounts
 * carrefull : it's not a copy, so we must not free or change it
 * if we want to change the list, use gsb_data_account_get_copy_list_accounts instead
 *
 * \param none
 *
 * \return a g_slist on the accounts
 **/
GSList *gsb_data_account_get_list_accounts (void)
{
    return list_accounts;
}

/**
 * create a new account and add to the list of accounts
 *
 * \param account_type the type of the account
 *
 * \return no of account, -1 if problem
 **/
gint gsb_data_account_new (KindAccount account_kind)
{
    AccountStruct *account;
    gint last_number;

    account = g_malloc0 (sizeof (AccountStruct));
    if (!account)
    {
		dialogue_error_memory ();

		return -1;
    }

    last_number = gsb_data_account_max_number ();

	/* we have to append the account first because some functions later will
     * look for that account */
    list_accounts = g_slist_append (list_accounts, account);

    /* set the base */
    account->account_number = last_number + 1;
    account->account_name = g_strdup_printf (_("No name %d"), account->account_number);
    account->currency = gsb_data_currency_get_default_currency ();

    /* set the kind of account */
    account->account_kind = account_kind;

    /* select the white line */
    account->current_transaction_number = -1;

    /* if it's the first account, we set default conf (R not displayed and 3 lines per transaction) */
    /* else we keep the conf of the last account same for sorting the transactions list */

    if (account->account_number == 1)
    {
		account->nb_rows_by_transaction = 3;

		/* sort the transactions by default */
		gsb_data_account_set_default_sort_values (account->account_number);
    }
    else
    {
		account->show_r = gsb_data_account_get_r (last_number);
		account->nb_rows_by_transaction = gsb_data_account_get_nb_rows (last_number);

		/* try to copy the sort values of the last account, else set default values */
		if (!gsb_data_account_dup_sort_values (last_number, account->account_number))
			gsb_data_account_set_default_sort_values (account->account_number);
    }

    return account->account_number;
}

/**
 * delete and free the account given
 *
 * \param account_number the no of account
 *
 * \return TRUE if ok
 **/
gboolean gsb_data_account_delete (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    list_accounts = g_slist_remove (list_accounts, account);
    _gsb_data_account_free (account);

    return TRUE;
}

/**
 * returns the number of accounts
 *
 * \param none
 *
 * \return amount of accounts
 **/
gint gsb_data_account_get_number_of_accounts (void)
{
    if (!list_accounts)
		return 0;

    return g_slist_length (list_accounts);
}

/**
 * find and return the first number of account
 *
 * \param none
 *
 * \return first number of account, -1 if no accounts
 **/
gint gsb_data_account_first_number (void)
{
    AccountStruct *account;

    if (!list_accounts)
        return -1;

    account = list_accounts->data;

    return  account->account_number;
}

/**
 * ind and return the first no closed account
 *
 * \param none
 *
 * \return first number of account, -1 if no accounts
 **/
gint gsb_data_account_first_no_closed_account (void)
{
    AccountStruct *account = NULL;
    GSList *list_tmp;

    if (!list_accounts)
        return -1;

    list_tmp = list_accounts;
    while (list_tmp)
    {
        account = list_tmp->data;
        if (account->closed_account == 0)
            break;

        list_tmp = list_tmp->next;
    }

    return  account->account_number;
}

/**
 * find and return the number of the account which the struct is the param
 *
 * \param the struct of the account
 *
 * \return the number of account, -1 if pb
 **/
gint gsb_data_account_get_no_account (gpointer account_ptr)
{
    AccountStruct *account;

    if (!account_ptr)
		return -1;

    account = account_ptr;
    account_buffer = account;

    return  account->account_number;
}

/**
 * change the number of the account given in param
 * it returns the new number (given in param also)
 * it is called ONLY when loading a file to change the default
 * number, given when we create the account
 *
 * \param account_number no of the account to change
 * \param new_no new number to the account
 *
 * \return the new number, or -1 if failed
 **/
gint gsb_data_account_set_account_number (gint account_number,
										  gint new_no)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return -1;

    account->account_number = new_no;

    return new_no;
}

/**
 * get the nb of rows displayed on the account given
 *
 * \param account_number no of the account
 *
 * \return nb of rows displayed (1, 2, 3 or 4), or 0 if the account doesn't exist
 **/
gint gsb_data_account_get_nb_rows (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->nb_rows_by_transaction;
}

/**
 * set the nb of rows displayed in the account given
 *
 * \param account_number no of the account
 * \param nb_rows number of rows per transaction (1, 2, 3 or 4)
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_nb_rows (gint account_number,
									   gint nb_rows)
{
    AccountStruct *account;

    if (nb_rows < 1 || nb_rows > 4)
    {
		g_print (_("Bad nb rows to gsb_data_account_set_nb_rows in gsb_data_account.c: %d\n"),
				 nb_rows);

		return FALSE;
    }

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->nb_rows_by_transaction = nb_rows;

    return TRUE;
}

/**
 * return if archives Lines are displayed in the account asked
 *
 * \param account_number no of the account
 *
 * \return boolean show/not show L
 **/
gboolean gsb_data_account_get_l (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->show_l;
}

/**
 * set if archives lines are displayed in the account asked
 *
 * \param account_number no of the account
 * \param show_l boolean
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_l (gint account_number,
								 gboolean show_l)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return FALSE;

    account->show_l = show_l;

	return TRUE;
}

/**
 * return if R are displayed in the account asked
 *
 * \param account_number no of the account
 *
 * \return boolean show/not show R
 **/
gboolean gsb_data_account_get_r (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->show_r;
}

/**
 * set if R are displayed in the account asked
 *
 * \param account_number no of the account
 * \param show_r boolean
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_r (gint account_number,
								 gboolean show_r)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->show_r = show_r;

    return TRUE;
}

/**
 * get the id of the account
 *
 * \param account_number no of the account
 *
 * \return id or 0 if the account doesn't exist
 **/
gchar *gsb_data_account_get_id (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return NULL;

    return account->account_id;
}

/**
 * set the id of the account
 * the id is copied in memory
 *
 * \param account_number no of the account
 * \param id id to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_id (gint account_number,
								  const gchar *id)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    if (account->account_id)
        g_free (account->account_id);

    account->account_id = my_strdup (id);

    return TRUE;
}

/**
 * Test if account exist by id (// Modif Yoann)
 * and return its number
 *
 * \param Account Id
 *
 * \return the account number or -1
 */
gint gsb_data_account_get_account_by_id (const gchar *account_id)
{
    GSList *list_tmp;

    list_tmp = gsb_data_account_get_list_accounts ();
    while (list_tmp)
    {
        AccountStruct *account;

        account = list_tmp->data;
        if (account_id
			&& account->account_number >= 0
			&& !account->closed_account
			&& account->account_id
			&& strlen (account->account_id) > 0)
        {
            if (g_ascii_strcasecmp (account_id, account->account_id) == 0)
                return account->account_number;
        }
        list_tmp = list_tmp->next;
    }
    return -1;
}

/**
 * get the account kind of the account
 *
 * \param account_number no of the account
 *
 * \return account type or 0 if the account doesn't exist
 **/
KindAccount gsb_data_account_get_kind (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->account_kind;
}

/**
 * set the kind of the account
 *
 * \param account_number no of the account
 * \param account_kind type to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_kind (gint account_number,
									KindAccount account_kind)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->account_kind = account_kind;

    return TRUE;
}

/**
 * get the name of the account
 *
 * \param account_number no of the account
 *
 * \return name or NULL if the account doesn't exist
 **/
gchar *gsb_data_account_get_name (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return NULL;

    return account->account_name;
}

/**
 * set the name of the account
 * the name is copied in memory
 *
 * \param account_number no of the account
 * \param name name to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_name (gint account_number,
									const gchar *name)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    g_free (account->account_name);

    if (!name || !strlen (name))
		account->account_name = NULL;
    else
		account->account_name = my_strdup (name);

    return TRUE;
}

/**
 * find and return the number of account which
 * have the name given in param
 *
 * \param account_name
 *
 * \return the number of account or -1
 **/
gint gsb_data_account_get_no_account_by_name (const gchar *account_name)
{
    GSList *list_tmp;

    if (!account_name)
		return -1;

    list_tmp = list_accounts;
    while (list_tmp)
    {
		AccountStruct *account;

		account = list_tmp->data;
		if (!strcmp (account->account_name, account_name))
			return account->account_number;

		list_tmp = list_tmp->next;
    }

    return -1;
}

/**
 * get the init balance of the account
 *
 * \param account_number no of the account
 * \param floating_point give the number of digits after the separator we want, -1 for no limit
 *
 * \return balance or NULL if the account doesn't exist
 **/
GsbReal gsb_data_account_get_init_balance (gint account_number,
										   gint floating_point)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return null_real;

	/* fix bug 2149 si le nombre est en erreur on renvoie error_real et non null_real */
	if (account->init_balance.mantissa == G_MININT64)
		return account -> init_balance;
	else
		return gsb_real_adjust_exponent (account->init_balance, floating_point);
}

/**
 * set the init balance of the account
 *
 * \param account_number no of the account
 * \param balance balance to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_init_balance (gint account_number,
											GsbReal balance)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->init_balance = balance;
    account->balances_are_dirty = TRUE;

    return TRUE;
}

/**
 * get the minimum balance wanted of the account
 *
 * \param account_number no of the account
 *
 * \return balance or NULL if the account doesn't exist
 **/
GsbReal gsb_data_account_get_mini_balance_wanted (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	return null_real;

    return account->mini_balance_wanted;
}

/**
 * set the minimum balance wanted of the account
 *
 * \param account_number no of the account
 * \param balance balance to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_mini_balance_wanted (gint account_number,
												   GsbReal balance)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->mini_balance_wanted = balance;

    return TRUE;
}

/**
 * get the minimum balance authorized of the account
 *
 * \param account_number no of the account
 *
 * \return balance or 0 if the account doesn't exist
 **/
GsbReal gsb_data_account_get_mini_balance_authorized (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return null_real;

    return account->mini_balance_authorized;
}

/**
 * set the minimum balance authorized of the account
 *
 * \param account_number no of the account
 * \param balance balance to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_mini_balance_authorized (gint account_number,
													   GsbReal balance)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->mini_balance_authorized = balance;

    return TRUE;
}

/**
 * flag the current and marked balance dirty to force recompute
 *
 * \param account_number no of the account
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_balances_are_dirty (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->balances_are_dirty = TRUE;

    return TRUE;
}

/**
 * calculate and fill in the account the current and marked balance of that account
 * it's faster than calling gsb_data_account_privatecalculate_current_balance and
 * gsb_data_account_private_calculate_marked_balance because throw the list only one time
 * called especially to init that values
 * the value calculated will have the same exponent of the currency account
 *
 * \param account_number
 *
 * \return the current balance
 **/
GsbReal gsb_data_account_calculate_current_and_marked_balances (gint account_number)
{
    AccountStruct *account;
    GDate *date_jour;
    GSList *tmp_list;
    GsbReal current_balance;
	GsbReal current_balance_later = null_real;
    GsbReal marked_balance;
	GsbReal marked_balance_later = null_real;
    gint floating_point;
	gboolean has_pointed = FALSE;
	GrisbiAppConf *a_conf;

    /* devel_debug_int (account_number); */
    account = gsb_data_account_get_structure (account_number);

    if (!account)
        return null_real;

    floating_point = gsb_data_currency_get_floating_point (account->currency);

	/* fix bug 2149 si le nombre est en erreur on renvoie error_real et non null_real */
	if (account->init_balance.mantissa == G_MININT64)
	{
		account->current_balance = account->init_balance;
		account->marked_balance = account->init_balance;

		return account->current_balance;
	}
	else
	{
		current_balance = gsb_real_adjust_exponent (account->init_balance, floating_point);
		marked_balance = gsb_real_adjust_exponent (account->init_balance, floating_point);
	}

    date_jour = gdate_today ( );

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

	tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
		gint transaction_number;
		gint res = 0;

		transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);

		/* on regarde si on tient compte ou pas des échéances pour les soldes */
		if (a_conf->balances_with_scheduled)
			res = 0;
		else
		{
			const GDate *date;

			date = gsb_data_transaction_get_value_date_or_date (transaction_number);
			if (date && g_date_valid (date))
				res = g_date_compare (date_jour, date);
		}

		if (gsb_data_transaction_get_account_number (transaction_number) == account_number
			 && !gsb_data_transaction_get_mother_transaction_number (transaction_number)
			 && res >= 0)
		{
			gint marked_transaction;
			GsbReal adjusted_amout;
			GsbReal tmp_balance;

			adjusted_amout = gsb_data_transaction_get_adjusted_amount (transaction_number, floating_point);
			tmp_balance = gsb_real_add (current_balance, adjusted_amout);
			if(tmp_balance.mantissa != error_real.mantissa)
			{
				current_balance = tmp_balance;
			}
			else
			{
				current_balance_later = gsb_real_add (current_balance_later, adjusted_amout);
			}
			marked_transaction = gsb_data_transaction_get_marked_transaction (transaction_number);
			if (marked_transaction)
			{
				tmp_balance = gsb_real_add (marked_balance, adjusted_amout);
				if(tmp_balance.mantissa != error_real.mantissa)
					marked_balance = tmp_balance;
				else
					marked_balance_later = gsb_real_add (marked_balance_later, adjusted_amout);
				if (marked_transaction == OPERATION_POINTEE)
					has_pointed = TRUE;
			}
		}
		tmp_list = tmp_list->next;
    }

    g_date_free (date_jour);
    account->current_balance = gsb_real_add (current_balance, current_balance_later);
    account->marked_balance = gsb_real_add (marked_balance, marked_balance_later);
	account->has_pointed = has_pointed;

    return account->current_balance;
}

/**
 * get the current balance  of the account
 *
 * \param account_number no of the account
 *
 * \return balance or 0 if the account doesn't exist
 **/
GsbReal gsb_data_account_get_current_balance (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return null_real;

    if (account->balances_are_dirty)
    {
        gsb_data_account_calculate_current_and_marked_balances (account_number);
        account->balances_are_dirty = FALSE;
    }
    return account->current_balance;
}

/**
 * get the marked balance of the account
 * this is the total of all marked transactions (R, P and T)
 *
 * \param account_number no of the account
 *
 * \return balance or 0 if the account doesn't exist
 **/
GsbReal gsb_data_account_get_marked_balance (gint account_number)
{
    AccountStruct *account;

	account = gsb_data_account_get_structure (account_number);
    if (!account)
        return null_real;

    if(account->balances_are_dirty)
    {
        gsb_data_account_calculate_current_and_marked_balances(account_number);
        account->balances_are_dirty = FALSE;
    }
    return account->marked_balance;
}

/**
 * calculate the amount of the marked T and P transactions, don't take care of R transactions
 * the value calculated will have the same exponent of the currency account
 *
 * \param account_number
 *
 * \return the amount
 **/
GsbReal gsb_data_account_calculate_waiting_marked_balance (gint account_number)
{
    AccountStruct *account;
    GSList *tmp_list;
    GsbReal marked_balance = null_real;
    gint floating_point;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return null_real;

    floating_point = gsb_data_currency_get_floating_point (account->currency);
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
		gint transaction_number;

		transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
		if (gsb_data_transaction_get_account_number (transaction_number) == account_number
			&& !gsb_data_transaction_get_mother_transaction_number (transaction_number)
			&& (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_POINTEE
			   ||
			   gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_TELEPOINTEE))
			marked_balance = gsb_real_add (marked_balance,
										   gsb_data_transaction_get_adjusted_amount (transaction_number,
																					 floating_point));
		tmp_list = tmp_list->next;
    }

	return marked_balance;
}

/**
 * get the element number used to sort the list in a column
 *
 * \param account_number no of the account
 * \param no_column no of the column
 *
 * \return  the element_number used to sort or 0 if the account doesn't exist
 **/
gint gsb_data_account_get_element_sort (gint account_number,
										gint no_column)
{
    AccountStruct *account;

    if (no_column < 0 || no_column >= CUSTOM_MODEL_VISIBLE_COLUMNS)
    {
        gchar* tmp_str;

    	/* TODO dOm : the return value of g_strdup_printf was not used !
         I add the devel_debug to print it. Is it OK to do that ? */
        tmp_str = g_strdup_printf (_("Bad no column to gsb_data_account_get_element_sort () "
									 "in data_account.c\nno_column = %d\n"),
								   no_column);
        g_print ("%s", tmp_str);
        g_free(tmp_str);

        return FALSE;
    }

    account = gsb_data_account_get_structure (account_number);

    if (!account)
		return 0;

    return account->column_element_sort[no_column];
}

/**
 * set the element number used to sort the column given in param
 *
 * \param account_number no of the account
 * \param no_column no of the column
 * \param element_number  element number used to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_element_sort (gint account_number,
											gint no_column,
											gint element_number)
{
    AccountStruct *account;

    if (no_column < 0 || no_column >= CUSTOM_MODEL_VISIBLE_COLUMNS)
    {
        gchar* tmp_str;

        /* TODO dOm : the value of g_strdup_printf was not used.
         I add the devel_debug function to print it. Is it OK ? */
        tmp_str = g_strdup_printf (_("Bad no column to gsb_data_account_set_element_sort () "
                                  "in data_account.c\nno_column = %d\n"),
                                  no_column);
        g_print ("%s", tmp_str);
		g_free (tmp_str);

		return FALSE;
    }

    account = gsb_data_account_get_structure (account_number);
    if (account == NULL)
        return FALSE;

    account->column_element_sort[no_column] = element_number;

    return TRUE;
}

/**
 * get the number of the current transaction in the given account
 *
 * \param account_number
 *
 * \return the number of the transaction or 0 if problem
 **/
gint gsb_data_account_get_current_transaction_number (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->current_transaction_number;
}

/**
 * set the current transaction of the account
 *
 * \param account_number no of the account
 * \param transaction_number number of the transaction selection
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_current_transaction_number (gint account_number,
														  gint transaction_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->current_transaction_number = transaction_number;

    return TRUE;
}

/**
 * get the value of mini_balance_wanted_message  on the account given
 *
 * \param account_number no of the account
 *
 * \return mini_balance_wanted_message or 0 if the account doesn't exist
 **/
gboolean gsb_data_account_get_mini_balance_wanted_message (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->mini_balance_wanted_message;
}

/**
 * set the value of mini_balance_wanted_message in the account given
 *
 * \param account_number no of the account
 * \param value
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_mini_balance_wanted_message (gint account_number,
														   gboolean value)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->mini_balance_wanted_message = value;

    return TRUE;
}

/**
 * get the value of mini_balance_authorized_message  on the account given
 *
 * \param account_number no of the account
 *
 * \return mini_balance_authorized_message or 0 if the account doesn't exist
 **/
gboolean gsb_data_account_get_mini_balance_authorized_message (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->mini_balance_authorized_message;
}

/**
 * set the value of mini_balance_authorized_message in the account given
 *
 * \param account_number no of the account
 * \param value
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_mini_balance_authorized_message (gint account_number,
															   gboolean value)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->mini_balance_authorized_message = value;

    return TRUE;
}

/**
 * get the currency on the account given
 *
 * \param account_number no of the account
 *
 * \return currency or 0 if the account doesn't exist
 **/
gint gsb_data_account_get_currency (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->currency;
}

/**
 * set the currency in the account given
 *
 * \param account_number no of the account
 * \param currency the currency to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_currency (gint account_number,
										gint currency)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->currency = currency;

    return TRUE;
}

/**
 * get the bank on the account given
 *
 * \param account_number no of the account
 *
 * \return last number of reconciliation or 0 if the account doesn't exist
 **/
gint gsb_data_account_get_bank (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->bank_number;
}

/**
 * set the bank in the account given
 *
 * \param account_number no of the account
 * \param bank the bank to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_bank (gint account_number,
									gint bank)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->bank_number = bank;

    return TRUE;
}

/**
 * get the bank_branch_code of the account
 *
 * \param account_number no of the account
 *
 * \return id or NULL if the account doesn't exist
 **/
gchar *gsb_data_account_get_bank_branch_code (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return NULL;

    return account->bank_branch_code;
}

/**
 * set the bank_branch_code of the account
 * the code is copied in memory
 *
 * \param account_number no of the account
 * \param bank_branch_code bank_branch_code to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_bank_branch_code (gint account_number,
												const gchar *bank_branch_code)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    g_free (account->bank_branch_code);

    if (!bank_branch_code || !strlen (bank_branch_code))
		account->bank_branch_code = NULL;
    else
		account->bank_branch_code = my_strdup (bank_branch_code);

    return TRUE;
}

/**
 * get the bank_account_number of the account
 *
 * \param account_number no of the account
 *
 * \return id or NULL if the account doesn't exist
 **/
gchar *gsb_data_account_get_bank_account_number (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return NULL;

    return account->bank_account_number;
}

/**
 * set the bank_account_number of the account
 * the number is copied in memory
 *
 * \param account_number no of the account
 * \param bank_account_number bank_account_number to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_bank_account_number (gint account_number,
												   const gchar *bank_account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    if (account->bank_account_number)
        g_free (account->bank_account_number);

    if (!bank_account_number || !strlen (bank_account_number))
		account->bank_account_number = NULL;
    else
		account->bank_account_number = my_strdup (bank_account_number);

    return TRUE;
}

/**
 * get the bank_account_key of the account
 *
 * \param account_number no of the account
 *
 * \return id or NULL if the account doesn't exist
 **/
gchar *gsb_data_account_get_bank_account_key (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return NULL;

    return account->bank_account_key;
}

/**
 * set the bank_account_key of the account
 * the key is copied in memory
 *
 * \param account_number no of the account
 * \param bank_account_key bank_account_key to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_bank_account_key (gint account_number,
												const gchar *bank_account_key)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    if (account->bank_account_key)
        g_free (account->bank_account_key);

    if (!bank_account_key || !strlen (bank_account_key))
		account->bank_account_key = NULL;
    else
		account->bank_account_key = my_strdup (bank_account_key);

    return TRUE;
}

/**
 * get closed_account on the account given
 *
 * \param account_number no of the account
 *
 * \return true if account is closed
 **/
gint gsb_data_account_get_closed_account (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->closed_account;
}

/**
 * set closed_account in the account given
 *
 * \param account_number no of the account
 * \param closed_account closed_account to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_closed_account (gint account_number,
											  gint closed_account)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->closed_account = closed_account;

    return TRUE;
}

/**
 * get the comment of the account
 *
 * \param account_number no of the account
 *
 * \return comment or NULL if the account doesn't exist
 **/
gchar *gsb_data_account_get_comment (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return NULL;

    return account->comment;
}

/**
 * set the comment of the account
 * the comment is copied in memory
 *
 * \param account_number no of the account
 * \param comment comment to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_comment (gint account_number,
									   const gchar *comment)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    g_free (account->comment);
    account->comment = my_strdup (comment);

    return TRUE;
}

/**
 * get reconcile_sort_type on the account given
 * 1 if the list should be sorted by method of payment, 0 if normal sort
 *
 * \param account_number no of the account
 *
 * \return 1 if the list should be sorted by method of payment, 0 if normal sort
 **/
gint gsb_data_account_get_reconcile_sort_type (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->reconcile_sort_type;
}

/**
 * set reconcile_sort_type in the account given
 * 1 if the list should be sorted by method of payment, 0 if normal sort
 *
 * \param account_number no of the account
 * \param sort_type
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_reconcile_sort_type (gint account_number,
												   gint sort_type)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->reconcile_sort_type = sort_type;

    return TRUE;
}

/**
 * get the sort_list of the account
 * this is a sorted list containing the numbers of the method of payment
 * used to sort the list while reconciling, according to the method of payments
 *
 * \param account_number no of the account
 *
 * \return the g_slist or NULL if the account doesn't exist
 **/
GSList *gsb_data_account_get_sort_list (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return NULL;

    return account->sort_list;
}

/**
 * set the sort_list list of the account
 * this is a sorted list containing the numbers of the method of payment
 * used to sort the list while reconciling, according to the method of payments
 *
 * \param account_number no of the account
 * \param list g_slist to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_sort_list (gint account_number,
										 GSList *list)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->sort_list = list;

    return TRUE;
}

/**
 * add a number of method of payment to the sort list of the account
 *
 * \param account_number
 * \param payment_number a gint which is the number of method of payment
 *
 * \return TRUE ok, FALSE problem
 **/
gboolean gsb_data_account_sort_list_add (gint account_number,
										 gint payment_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->sort_list = g_slist_append (account->sort_list, GINT_TO_POINTER (payment_number));

    return TRUE;
}

/**
 * remove a number of method of payment to the sort list of the account
 *
 * \param account_number
 * \param payment_number a gint which is the number of method of payment
 *
 * \return TRUE ok, FALSE problem
 **/
gboolean gsb_data_account_sort_list_remove (gint account_number,
											gint payment_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->sort_list = g_slist_remove (account->sort_list, GINT_TO_POINTER (payment_number));

    return TRUE;
}

/**
 * free the sort list of the account
 *
 * \param account_number
 *
 * \return TRUE ok, FALSE problem
 **/
gboolean gsb_data_account_sort_list_free (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    if (!account->sort_list)
		return TRUE;

    g_slist_free (account->sort_list);
    account->sort_list = NULL;

	return TRUE;
}

/**
 * get split_neutral_payment on the account given
 *
 * \param account_number no of the account
 *
 * \return split_neutral_payment or 0 if the account doesn't exist
 **/
gint gsb_data_account_get_split_neutral_payment (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->split_neutral_payment;
}

/**
 * set split_neutral_payment in the account given
 *
 * \param account_number no of the account
 * \param split_neutral_payment split_neutral_payment to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_split_neutral_payment (gint account_number,
													 gint split_neutral_payment)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->split_neutral_payment = split_neutral_payment;

    return TRUE;
}

/**
 * get the holder_name of the account
 *
 * \param account_number no of the account
 *
 * \return holder_name or NULL if the account doesn't exist
 **/
gchar *gsb_data_account_get_holder_name (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return NULL;

    return account->holder_name;
}

/**
 * set the holder_name of the account
 * the name is copied in memory
 *
 * \param account_number no of the account
 * \param holder_name holder_name to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_holder_name (gint account_number,
										   const gchar *holder_name)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    g_free (account->holder_name);

    if (!holder_name || !strlen (holder_name))
		account->holder_name = NULL;
    else
		account->holder_name = my_strdup (holder_name);

    return TRUE;
}

/**
 * get the holder_address of the account
 *
 * \param account_number no of the account
 *
 * \return holder_address or NULL if the account doesn't exist
 **/
gchar *gsb_data_account_get_holder_address (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return NULL;

    return account->holder_address;
}

/**
 * set the holder_address of the account
 * the address is copied in memory
 *
 * \param account_number no of the account
 * \param holder_address holder_address to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_holder_address (gint account_number,
											  const gchar *holder_address)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    g_free (account->holder_address);

    if (!holder_address || !strlen (holder_address))
		account->holder_address = NULL;
    else
		account->holder_address = my_strdup (holder_address);

    return TRUE;
}

/**
 * get default_debit on the account given
 *
 * \param account_number no of the account
 *
 * \return default_debit or 0 if the account doesn't exist
 **/
gint gsb_data_account_get_default_debit (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->default_debit;
}

/**
 * set default_debit in the account given
 *
 * \param account_number no of the account
 * \param default_debit default_debit to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_default_debit (gint account_number,
											 gint default_debit)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->default_debit = default_debit;

    return TRUE;
}

/**
 * get default_credit on the account given
 *
 * \param account_number no of the account
 *
 * \return default_credit or 0 if the account doesn't exist
 **/
gint gsb_data_account_get_default_credit (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->default_credit;
}

/**
 * set default_credit in the account given
 *
 * \param account_number no of the account
 * \param default_credit default_credit to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_default_credit (gint account_number,
											  gint default_credit)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->default_credit = default_credit;

    return TRUE;
}

/**
 * get row_align on the account given
 *
 * \param account_number no of the account
 *
 * \return the row_align or 0
 **/
gfloat gsb_data_account_get_row_align (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0.0;

    return account->row_align;
}

/**
 * set the row_align parameter for the account
 * use to place the list at the good place when changing account
 *
 * \param account_number no of the account
 * \param row_align	the row_align to use with gtk_tree_view_scroll_to_cell
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_row_align (gint account_number,
										 gfloat row_align)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->row_align = row_align;

	return TRUE;
}

/**
 * get sort_type on the account given
 * ie GTK_SORT_DESCENDING / GTK_SORT_ASCENDING
 *
 * \param account_number no of the account
 *
 * \return GTK_SORT_DESCENDING / GTK_SORT_ASCENDING
 **/
gint gsb_data_account_get_sort_type (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->sort_type;
}
/**
 * set sort_type in the account given
 * ie GTK_SORT_DESCENDING / GTK_SORT_ASCENDING
 *
 * \param account_number no of the account
 * \param sort_type sort_type to set (GTK_SORT_DESCENDING / GTK_SORT_ASCENDING)
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_sort_type (gint account_number,
										 gint sort_type)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->sort_type = sort_type;

    return TRUE;
}

/**
 * get sort_column on the account given
 *
 * \param account_number no of the account
 *
 * \return sort_column or 0 if the account doesn't exist
 **/
gint gsb_data_account_get_sort_column (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return 0;

    return account->sort_column;
}

/**
 * set sort_column in the account given
 *
 * \param account_number no of the account
 * \param sort_column sort_column to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_sort_column (gint account_number,
										   gint sort_column)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    account->sort_column = sort_column;

    return TRUE;
}

/**
 * set a new order in the list of accounts
 * all the accounts which are not in the new order are appened at the end of the new list
 * should be used only when loading a file before the 0.6 version
 *
 * \param new_order a g_slist which contains the number of accounts in the new order
 *
 * \return FALSE
 **/
gboolean gsb_data_account_reorder (GSList *new_order)
{
    GSList *last_list;
    GSList *list_tmp;
    GSList *new_list_accounts = NULL;

    while (new_order)
    {
		new_list_accounts = g_slist_append (new_list_accounts,
											gsb_data_account_get_structure (GPOINTER_TO_INT (new_order->data)));
		new_order = new_order->next;
    }

    last_list = list_accounts;
    list_accounts = new_list_accounts;

    /* now we go to check if all accounts are in the list and append the at the end */
    list_tmp = last_list;

    while (list_tmp)
    {
		AccountStruct *account = list_tmp->data;

		if (!g_slist_find (list_accounts, account))
		{
			list_accounts = g_slist_append (list_accounts, account);
		}

		list_tmp = list_tmp->next;
    }

    g_slist_free (last_list);
    gsb_file_set_modified (TRUE);

    return TRUE;
}

/**
 * check the position of the 2 accounts in the list and
 * return -1 if first account before second (and +1 else)
 *
 * \param account_number_1
 * \param account_number_2
 *
 * \return -1 if account_number_1 before, account_number_2, and +1 else, 0 if one of account doesn't exist
 **/
gint gsb_data_account_compare_position (gint account_number_1,
										gint account_number_2)
{
    gint pos_1, pos_2;
    AccountStruct *account_1;
    AccountStruct *account_2;

    account_1 = gsb_data_account_get_structure (account_number_1);
    account_2 = gsb_data_account_get_structure (account_number_2);
    if (!account_1 || !account_2)
		return 0;

    pos_1 = g_slist_index (list_accounts, account_1);
    pos_2 = g_slist_index (list_accounts, account_2);
    if (pos_1 < pos_2)
		return -1;
    else
		return 1;
}

/**
 * change the position of an account in the list of accounts
 *
 * \param account_number	the account we want to move
 * \param dest_account_number	the account before we want to move, or -1 to set at the end of list
 *
 * \return FALSE
 **/
gboolean gsb_data_account_move_account (gint account_number,
										gint dest_account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
		return FALSE;

    /* first, remove the account from the list */
    list_accounts = g_slist_remove (list_accounts,  account);

    if (dest_account_number != -1)
    {
		GSList *tmp_list;

		tmp_list = list_accounts;
		while (tmp_list)
		{
			AccountStruct *account_tmp;

			account_tmp = tmp_list->data;
			if (account_tmp->account_number == dest_account_number)
			{
				list_accounts = g_slist_insert_before (list_accounts, tmp_list, account);

				return FALSE;
			}
			tmp_list = tmp_list->next;
		}
    }

    /* we move the account to the end */
    list_accounts = g_slist_append (list_accounts, account);

    return FALSE;
}

/**
 * get the icon_path of the account
 *
 * \param account_number no of the account
 *
 * \return icon_path or NULL if the account doesn't exist
 **/
const gchar *gsb_data_account_get_name_icon (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return NULL;

    return account->name_icon;
}

/**
 * set the icon_path of the account
 * the address is copied in memory
 *
 * \param account_number no of the account
 * \param filename name of file to set
 *
 * \return TRUE, ok ; FALSE, problem
 **/
gboolean gsb_data_account_set_name_icon (gint account_number,
										 const gchar *filename)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    g_free (account->name_icon);

    if (!filename || !strlen (filename))
        account->name_icon = NULL;
    else
    {
        if (!g_file_test (filename, G_FILE_TEST_EXISTS))
            account->name_icon = NULL;
        else
            account->name_icon = my_strdup (filename);
    }

    return TRUE;
}

/**
 * get the image icon of the account
 *
 * \param account_number no of the account
 *
 * \return image icon
 **/
GtkWidget *gsb_data_account_get_account_icon_image (gint account_number)
{
    GdkPixbuf * pixbuf;
    GtkWidget *image;

    pixbuf = gsb_data_account_get_account_icon_pixbuf (account_number);
    image = gtk_image_new_from_pixbuf (pixbuf);
    g_object_unref (G_OBJECT (pixbuf));

    return image;
}

/**
 * get the pixbuf icon of the account
 *
 * \param account_number no of the account
 *
 * \return pixbuf icon
 **/
GdkPixbuf *gsb_data_account_get_account_icon_pixbuf (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return NULL;

    if (account->pixbuf)
    {
        g_object_ref (account->pixbuf);

		return account->pixbuf;
    }
    else
    {
        GdkPixbuf *pixbuf = NULL;

        pixbuf = gsb_data_account_get_account_standard_pixbuf (account->account_kind);

        return pixbuf;
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
gboolean gsb_data_account_set_account_icon_pixbuf (gint account_number,
												   GdkPixbuf *pixbuf)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    if (account->pixbuf)
        g_object_unref (account->pixbuf);

    if (pixbuf)
        account->pixbuf = pixbuf;
    else
        account->pixbuf = NULL;

    return TRUE;
}

/**
 * get the default pixbuf icon for the kind_account
 *
 * \param kind_account
 *
 * \return pixbuf icon
 **/
GdkPixbuf *gsb_data_account_get_account_standard_pixbuf (KindAccount account_kind)
{
    GdkPixbuf * pixbuf = NULL;
    gchar *filename;
    GError *error = NULL;

    filename = gsb_data_account_get_account_standard_pixbuf_filename (account_kind);
    pixbuf = gdk_pixbuf_new_from_file (filename, &error);
    if (pixbuf)
        g_object_set_data_full (G_OBJECT (pixbuf), "name_icon", filename, g_free);
    else
        devel_debug (error->message);

    return pixbuf;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void gsb_data_account_change_account_icon (GtkWidget *button,
										   gpointer data)
{
    gchar *std_pixbuf_filename;
    const gchar *name_icon;
    gchar *new_icon;
    gint current_account;

    devel_debug (NULL);

    current_account = gsb_gui_navigation_get_current_account ();
    std_pixbuf_filename = gsb_data_account_get_account_standard_pixbuf_filename (gsb_data_account_get_kind
																				 (current_account));

    name_icon = gsb_data_account_get_name_icon (current_account);
    if (name_icon == NULL || g_file_test (name_icon, G_FILE_TEST_EXISTS) == FALSE)
        name_icon = std_pixbuf_filename;

    new_icon = gsb_select_icon_create_window (name_icon);

	if (new_icon && strcmp (new_icon, name_icon) != 0)
    {
    	GtkWidget *image;

		if (strcmp (new_icon, std_pixbuf_filename) == 0)
        {
            gsb_data_account_set_name_icon (current_account, NULL);
            gsb_data_account_set_account_icon_pixbuf (current_account, NULL);

			image = gsb_data_account_get_account_icon_image (current_account);
			gtk_button_set_image (GTK_BUTTON (button), image);
			gsb_gui_navigation_update_account (current_account);
        }
        else
		{
    		GdkPixbuf *pixbuf;

			gsb_data_account_set_name_icon ( current_account, new_icon );
            pixbuf = gsb_select_icon_new_account_pixbuf_from_file (new_icon);
            gsb_data_account_set_account_icon_pixbuf ( current_account, pixbuf );

			image = gsb_data_account_get_account_icon_image ( current_account );
			gtk_button_set_image ( GTK_BUTTON ( button ), image );
			gtk_widget_show (image);
			gsb_gui_navigation_update_account ( current_account );
        }

        gsb_file_set_modified (TRUE);
    }

    g_free (std_pixbuf_filename);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gchar *gsb_data_account_get_bank_account_iban (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return NULL;

    return account->bank_account_iban;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bank_account_iban (gint account_number,
												 const gchar *iban)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    g_free (account->bank_account_iban);

    if (!iban || !strlen (iban))
        account->bank_account_iban = NULL;
    else
        account->bank_account_iban = my_strdup (iban);

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_data_account_colorize_current_balance (gint account_number)
{
    gchar *string;
    gchar *tmp_str;

    tmp_str = utils_real_get_string_with_currency (gsb_data_account_get_current_balance (account_number),
												  gsb_data_account_get_currency (account_number),
												  TRUE);
    string = tmp_str;
    if (gsb_data_account_get_current_balance (account_number).mantissa < 0)
    {
        string = g_strdup_printf ("<span color=\"red\">%s</span>", tmp_str);
        g_free (tmp_str);
    }

    if (!string)
        string = g_strdup ("");

    grisbi_win_headings_update_suffix (string);
    g_free (string);
}

/**
 * Calculates the balance at the date today for the bet module.
 * Excludes future transactions.
 *
 * \param
 * \param
 *
 * \return
 **/
GsbReal gsb_data_account_calculate_current_day_balance (gint account_number,
														GDate *day)
{
    AccountStruct *account;
    GDate *date_jour;
    GSList *tmp_list;
    GsbReal current_balance;
	GsbReal current_balance_later = null_real;
    gint floating_point;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return null_real;

    floating_point = gsb_data_currency_get_floating_point (account->currency);
    current_balance = gsb_real_adjust_exponent (account->init_balance, floating_point);
    if (day == NULL)
        date_jour = gdate_today ();
    else
        date_jour = gsb_date_copy (day);

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
        gint transaction_number;
        gint res = 0;
        GsbReal adjusted_amout;
        GsbReal tmp_balance;

        transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);

        /* on ne tient pas compte des échéances futures pour le solde */
        res = g_date_compare (date_jour,
							  gsb_data_transaction_get_date (transaction_number));
        if (gsb_data_transaction_get_account_number (transaction_number) == account_number
             && !gsb_data_transaction_get_mother_transaction_number (transaction_number)
             && res > 0)
        {
            adjusted_amout = gsb_data_transaction_get_adjusted_amount (transaction_number, floating_point);
            tmp_balance = gsb_real_add (current_balance, adjusted_amout);

            if(tmp_balance.mantissa != error_real.mantissa)
                current_balance = tmp_balance;
            else
                current_balance_later = gsb_real_add (current_balance_later, adjusted_amout);
        }
        tmp_list = tmp_list->next;
    }

    g_date_free (date_jour);

    return gsb_real_add (current_balance, current_balance_later);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GDate *gsb_data_account_get_bet_start_date (gint account_number)
{
    GDate *date = NULL;
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return 0;

    date = account->bet_start_date;

    if (date && g_date_valid (date))
        return gsb_date_copy (date);
    else
    {
        date = gdate_today ();
        if (etat.bet_debut_period == 1)
            g_date_set_day (date, 1);

        return date;
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
gboolean gsb_data_account_set_bet_start_date (gint account_number,
											  const GDate *date)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return FALSE;

    if (date && g_date_valid (date))
    {
        account->bet_start_date = gsb_date_copy (date);

		return TRUE;
    }
    else
    {
        GDate *date_new;

        date_new = gdate_today ();
        if (etat.bet_debut_period == 1)
            g_date_set_day (date_new, 1);
        account->bet_start_date = date_new;

		return FALSE;
    }
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_account_get_bet_spin_range (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return 0;

    return account->bet_spin_range;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_spin_range (gint account_number,
											  gint spin_range)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return FALSE;

    account->bet_spin_range = spin_range;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_account_get_bet_months (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return 0;

    if (account->bet_months == 0)
        return 1;
    else
        return account->bet_months;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_months (gint account_number,
										  gint months)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->bet_months = months;

    return TRUE;
}

/**
 * return the historical source of the data
 *
 * \param
 *
 * \return
 **/
gint gsb_data_account_get_bet_hist_data (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return 0;

    return account->bet_hist_data;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_hist_data (gint account_number,
											 gint hist_data)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return FALSE;

    account->bet_hist_data = hist_data;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_account_get_bet_hist_fyear (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return 0;

    return account->bet_hist_fyear;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_hist_fyear (gint account_number,
											  gint hist_fyear)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return FALSE;

    account->bet_hist_fyear = hist_fyear;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_account_get_bet_hist_use_data_in_account (gint account_number)
{
	AccountStruct *account;

	account = gsb_data_account_get_structure (account_number);
	if (!account)
		return 0;

	return account->bet_hist_use_data_in_account;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_hist_use_data_in_account (gint account_number,
															gint bet_hist_use_data_in_account)
{
	AccountStruct *account;
	account = gsb_data_account_get_structure (account_number);
	if (!account)
		return FALSE;

	account->bet_hist_use_data_in_account = bet_hist_use_data_in_account;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_get_bet_auto_inc_month (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return FALSE;

    return account->bet_auto_inc_month;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_auto_inc_month (gint account_number,
												  gboolean auto_inc_month)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return FALSE;

    account->bet_auto_inc_month = auto_inc_month;

    return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gint gsb_data_account_get_bet_select_label (gint account_number,
											gint origine)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return 0;

    switch (origine)
    {
        case SPP_ORIGIN_TRANSACTION:
            return account->bet_select_transaction_label;
            break;
        case SPP_ORIGIN_SCHEDULED:
            return account->bet_select_scheduled_label;
            break;
        case SPP_ORIGIN_FUTURE:
            return account->bet_select_futur_label;
            break;
    }

    return 0;
}

/**
 *
 *
 * \param
 * \param origine	SPP_ORIGIN_TRANSACTION, SPP_ORIGIN_SCHEDULED, SPP_ORIGIN_FUTURE
 * \param type		0 default (Note), 1 = Payees, 2= Categories, 3 = IB
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_select_label (gint account_number,
												gint origine,
												gint type)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);

    if (!account)
	    return FALSE;

    switch (origine)
    {
        case SPP_ORIGIN_TRANSACTION:
            account->bet_select_transaction_label = type;
            break;
        case SPP_ORIGIN_SCHEDULED:
            account->bet_select_scheduled_label = type;
            break;
        case SPP_ORIGIN_FUTURE:
            account->bet_select_futur_label = type;
            break;
    }

    return TRUE;
}


/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_bet_update_initial_date_if_necessary (gint account_number)
{
    GDate *date_jour;
    GDate *tmp_date;

    date_jour = gdate_today ();
    tmp_date = gsb_data_account_get_bet_start_date (account_number);
    g_date_add_months (tmp_date, 1);

    if (g_date_compare (date_jour, tmp_date) >= 0)
    {
        if (g_date_get_month (date_jour) == g_date_get_month (tmp_date))
            gsb_data_account_set_bet_start_date (account_number, tmp_date);
        else
        {
            g_date_set_day (date_jour, g_date_get_day (tmp_date));
            gsb_data_account_set_bet_start_date (account_number, date_jour);
        }
    }

    g_date_free (tmp_date);
    g_date_free (date_jour);

    return FALSE;
}

/**
 * retourne le bit utilisation du module budget.
 *
 * \param
 *
 * \return  -1 pas de module possible 0 non utilisé 1 utilisé
 **/
gint gsb_data_account_get_bet_use_budget (gint account_number)
{
    AccountStruct *account;
    KindAccount kind;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return 0;

    kind = account->account_kind;
    switch (kind)
    {
        case GSB_TYPE_BANK:
        case GSB_TYPE_CASH:
        case GSB_TYPE_LIABILITIES:
            return account->bet_use_budget;
            break;
        case GSB_TYPE_ASSET:
        case GSB_TYPE_BALANCE:
            return -1;
            break;
        default:
            return -1;
            break;
    }

    return -1;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_use_budget (gint account_number,
											  gint value)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
	    return FALSE;

    account->bet_use_budget = value;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_data_account_get_bet_maj (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return 0;

    return account->bet_maj;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_maj (gint account_number,
									   gint type_maj)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->bet_maj = type_maj;

    return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_finance_capital (gint account_number,
												   gdouble capital)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->bet_capital = capital;

    return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_finance_taux_annuel (gint account_number,
													   gdouble taux_annuel)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->bet_taux_annuel = taux_annuel;

    return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_finance_frais (gint account_number,
												 gdouble frais)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->bet_frais = frais;

    return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_finance_type_taux (gint account_number,
													 gint type_taux)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->bet_type_taux = type_taux;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_get_bet_init_sch_with_loan (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return 0;

    return account->bet_init_sch_with_loan;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_init_sch_with_loan (gint account_number,
													  gboolean init_sch_with_loan)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->bet_init_sch_with_loan = init_sch_with_loan;

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_get_bet_split_transaction (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return 0;

    return account->bet_split_transaction;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_set_bet_split_transaction (gint account_number,
													 gboolean split_transaction)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->bet_split_transaction = split_transaction;

    return TRUE;
}

/**
 * get floating_point of the currency on the account given
 *
 * \param account_number no of the account
 *
 * \return floating_point or 0 if the account or the currency doesn't exist
 **/
gint gsb_data_account_get_currency_floating_point (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
	if (!account)
		return 0;

	return gsb_data_currency_get_floating_point (account->currency);
}

/**
 * retourne l'option carte CB à débit différé.
 *
 * \param account_number
 *
 * \return -1 pas de CB possible 0 non utilisé 1 utilisé
 **/
gint gsb_data_account_get_bet_credit_card (gint account_number)
{
    AccountStruct *account;
    KindAccount kind;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return 0;

    kind = account->account_kind;
    switch (kind)
    {
        case GSB_TYPE_BANK:
        case GSB_TYPE_CASH:
        case GSB_TYPE_LIABILITIES:
            return account->bet_credit_card;
            break;
        case GSB_TYPE_ASSET:
        case GSB_TYPE_BALANCE:
            return 0;
            break;
        default:
            return 0;
            break;
    }

    return -1;
}

/**
 * positionne l'option bet_credit_card
 *
 * \param account_number
 * \param value
 *
 * \ return TRUE if OK
 **/
gboolean gsb_data_account_set_bet_credit_card (gint account_number,
											   gint value)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    account->bet_credit_card = value;

    return TRUE;
}

/**
 * retourne le type d'onglet à afficher pour le module budgetaire.
 *
 * \param account_number
 *
 * \return enum BetTypeOnglets
 **/
BetTypeOnglets gsb_data_account_get_bet_show_onglets (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return BET_ONGLETS_SANS;

    return account->bet_show_onglets;
}

/**
 * positionne l'option bet_show_onglets
 *
 * \param account_number
 *
 * \ return TRUE if OK
 **/
gboolean gsb_data_account_set_bet_show_onglets (gint account_number)
{
    AccountStruct *account;
    gint bet_use_budget;
    KindAccount kind;
	GrisbiWinEtat *w_etat;

	account = gsb_data_account_get_structure (account_number);
	if (!account)
		return FALSE;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
    bet_use_budget = gsb_data_account_get_bet_use_budget (account_number);


    if (bet_use_budget == 0)
    {
        account->bet_show_onglets = BET_ONGLETS_SANS;
        return TRUE;
    }

    kind = gsb_data_account_get_kind (account_number);
    switch (kind)
    {
		case GSB_TYPE_BALANCE:
			account->bet_show_onglets = BET_ONGLETS_SANS;
			break;
		case GSB_TYPE_BANK:
			account->bet_show_onglets = BET_ONGLETS_PREV;
			break;
		case GSB_TYPE_CASH:
			if (w_etat->bet_cash_account_option == 1)
				account->bet_show_onglets = BET_ONGLETS_PREV;
			else
				account->bet_show_onglets = BET_ONGLETS_HIST;
			break;
		case GSB_TYPE_LIABILITIES:
			if (account->bet_credit_card)
			{
				if (w_etat->bet_cash_account_option == 1)
					account->bet_show_onglets = BET_ONGLETS_PREV;
				else
					account->bet_show_onglets = BET_ONGLETS_HIST;
			}
			else
				account->bet_show_onglets = BET_ONGLETS_CAP;
			break;
		case GSB_TYPE_ASSET:
			account->bet_show_onglets = BET_ONGLETS_SANS;
			break;
    }

    return TRUE;
}

/**
 * positionne l'option bet_show_onglets pour tous les comptes
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_data_account_set_bet_show_onglets_all_accounts (void)
{
    GSList *list_tmp;

    list_tmp = gsb_data_account_get_list_accounts ();
    while (list_tmp)
    {
        gint account_number;

        account_number = gsb_data_account_get_no_account (list_tmp->data);
        gsb_data_account_set_bet_show_onglets (account_number);

        list_tmp = list_tmp->next;
    }

    return FALSE;
}

/**
 * teste l'existence d'un compte
 *
 * \param account_number
 *
 * \ return TRUE if OK else FALSE
 **/
gboolean gsb_data_account_exists (gint account_number)
{
    AccountStruct *account;

	account = gsb_data_account_get_structure (account_number);
    if (account)
        return TRUE;
    else
        return FALSE;
}

/**
 * calcule le solde d'un compte à une date donnée
 *
 * \param account_number    numéro du compte concerné
 * \param date              date de calcul du solde
 *
 * \return GsbReal         le solde du compte
 **/
GsbReal gsb_data_account_get_balance_at_date (gint account_number,
											  GDate *date)
{
    AccountStruct *account;
    GSList *tmp_list;
    gint floating_point;
    GsbReal current_balance;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return null_real;

    floating_point = gsb_data_currency_get_floating_point (account->currency);
    current_balance = gsb_real_adjust_exponent (account->init_balance, floating_point);
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
        gint transaction_number;

        transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);

        if (gsb_data_transaction_get_account_number (transaction_number) != account_number)
        {
            tmp_list = tmp_list->next;
            continue;
        }

        if (g_date_compare (gsb_data_transaction_get_value_date_or_date (transaction_number),
         date) > 0)
        {
            tmp_list = tmp_list->next;
            continue;
        }

        if (gsb_data_transaction_get_mother_transaction_number (transaction_number) == 0)
        {
            GsbReal adjusted_amout;
            GsbReal tmp_balance;

            adjusted_amout = gsb_data_transaction_get_adjusted_amount (transaction_number, floating_point);
            tmp_balance = gsb_real_add (current_balance, adjusted_amout);

            if (tmp_balance.mantissa == error_real.mantissa)
                return error_real;
            else
                current_balance = tmp_balance;
        }
        tmp_list = tmp_list->next;
    }

    return current_balance;
}

/**
 * set limits of current and authorized balance of all accounts
 *
 * \param
 *
 * \return
 **/
void gsb_data_account_set_all_limits_of_balance (void)
{
    GSList *tmp_list;

    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
		gint account_number;
		volatile gint value;

		account_number = gsb_data_account_get_no_account (tmp_list->data);

		/* set the minimum balances to be shown or not */
		value = gsb_real_cmp (gsb_data_account_get_current_balance (account_number),
							  gsb_data_account_get_mini_balance_authorized (account_number)) == -1;
		gsb_data_account_set_mini_balance_authorized_message (account_number, value);
		value = gsb_real_cmp (gsb_data_account_get_current_balance (account_number),
							  gsb_data_account_get_mini_balance_wanted (account_number)) == -1;
		gsb_data_account_set_mini_balance_wanted_message (account_number, value);

		tmp_list = tmp_list->next;
    }
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_get_has_pointed (gint account_number)
{
    AccountStruct *account;

    account = gsb_data_account_get_structure (account_number);
    if (!account)
        return FALSE;

    return account->has_pointed;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_data_account_renum_account_number_0 (const gchar *filename)
{
	gint account_number;
	GSList *payment_list;
    GSList *tmp_list;
	GSList *number_list = NULL;
	gchar* tmp_str;
	gint i = 0;
	gint result;

	devel_debug (NULL);

	/* recherche le premier numéro de compte disponible */
	tmp_list = list_accounts;
	while (tmp_list)
	{
		AccountStruct *account;

		account = tmp_list->data;
		if (!account)
		{
			return FALSE;
		}
		else
		{
			account_number = account->account_number;
		}
		number_list = g_slist_append (number_list, GINT_TO_POINTER (account_number));

		tmp_list = tmp_list->next;
    }

	number_list = g_slist_sort (number_list, (GCompareFunc) gsb_data_account_cmp_numbers);
	tmp_list = number_list;
    while (tmp_list)
    {
		account_number = GPOINTER_TO_INT (tmp_list->data);
		if (account_number > i)
		{
			break;
		}
		i++;
		tmp_list = tmp_list->next;
	}

	account_number = i;
	g_slist_free (number_list);

	/* Avertissement avant renommage du compte */
	tmp_str = g_strdup_printf (_("The account \"%s\" has the number 0 which creates a bug in certain situations."
								  " It will be renumbered and will have the number \"%d\"."),
							   gsb_data_account_get_name (0),
							   account_number);

	result = dialogue_yes_no (tmp_str, _("Rename account \"0\""), GTK_RESPONSE_YES);
    g_free (tmp_str);
	if (!result)
	{
		return FALSE;
    }

	/* on fait une sauvegarde du fichier */
	gsb_file_copy_old_file (filename);

	/* on traite les opérations */
	tmp_list = gsb_data_transaction_get_complete_transactions_list ();
	while (tmp_list)
	{
		gint transaction_number;

		transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
		if (gsb_data_transaction_get_account_number (transaction_number) == 0)
		{
			gsb_data_transaction_set_account_number (transaction_number, account_number);
		}
		tmp_list = tmp_list->next;
	}

	/* on traite les opérations planifiées*/
	tmp_list = gsb_data_scheduled_get_scheduled_list ();
	while (tmp_list)
	{
		gint scheduled_number;

		scheduled_number = gsb_data_scheduled_get_scheduled_number (tmp_list->data);
		if (gsb_data_scheduled_get_account_number (scheduled_number) == 0)
		{
			gsb_data_scheduled_set_account_number (scheduled_number, account_number);
		}
		tmp_list = tmp_list->next;
	}

	/* on traite les moyens de paiement */
	payment_list = gsb_data_payment_get_list_for_account (0);
	tmp_list = payment_list;
	while (tmp_list)
	{
		gint payment_number;

		payment_number = GPOINTER_TO_INT (tmp_list->data);
		gsb_data_payment_set_account_number (payment_number, account_number);
		tmp_list = tmp_list->next;
	}
	g_slist_free (payment_list);

	/* on traite les rapprochements */
	gsb_data_reconcile_renum_account_number_0 (account_number);

	/* on traite les règles d'import des fichiers */
	gsb_data_import_rule_renum_account_number_0 (account_number);

	/* on traite les états */
	gsb_data_report_renum_account_number_0 (account_number);

	/* on traite les données budgetaires */
	bet_data_renum_account_number_0 (account_number);

	/* On traite les soldes partiels */
	gsb_data_partial_balance_renum_account_number_0 (account_number);

	/* set new numero for account */
	gsb_data_account_set_account_number (0, account_number);

	gsb_file_set_modified (TRUE);

	return TRUE;
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
