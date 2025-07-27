/* ************************************************************************** */
/*     Copyright (C)	2004-2005 Alain Portal (aportal@univ-montp2.fr)	      */
/*                  	2006-2006 Benjamin Drieu (bdrieu@april.org)	          */
/*			https://www.grisbi.org/  			                              */
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

#include "config.h"

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_debug.h"
#include "dialog.h"
#include "grisbi_win.h"
#include "gsb_assistant.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_real.h"
#include "structures.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_real.h"
#include "utils_str.h"
#include "utils_widgets.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * fix the default
 *
 * \param function which fix the default
 *
 * \return FALSE
 **/
static gboolean gsb_debug_try_fix (gboolean (*fix) (void))
{

    if (fix ())
    {
        gsb_file_set_modified (TRUE);
		dialogue_hint (_("Grisbi successfully repaired this account file.  "
						 "You may now save your modifications."),
					   _("Fix completed"));
    }
    else
    {
		dialogue_error_hint (_("Grisbi was unable to repair this account file.  "
							   "No modification has been done."),
							 _("Unable to fix account"));
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
static void gsb_debug_add_report_page (GtkWidget *assistant,
									   gint page,
									   struct GsbDebugTest *test,
									   gchar *summary)
{
    GtkWidget *button;
    GtkWidget *label;
    GtkWidget *vbox;
    GtkWidget *scrolled_window;
    gchar *tmp_str;

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
    gtk_container_add (GTK_CONTAINER (scrolled_window), vbox);

    label = gtk_label_new (NULL);
    tmp_str = g_strconcat (dialogue_make_pango_attribut ("size=\"larger\" weight=\"bold\"", _(test->name)),
						   "\n\n",
						   summary,
						   NULL);
    gtk_label_set_markup (GTK_LABEL(label), tmp_str);
    gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
    gtk_label_set_justify (GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    utils_labels_set_alignment (GTK_LABEL (label), GSB_LEFT, GSB_BOTTOM);
    g_free (tmp_str);

    gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER(vbox), BOX_BORDER_WIDTH);

    if (test->instructions)
    {
		GtkWidget *expander;

		tmp_str = g_strconcat ("<b>",_("Details"), "</b>", NULL);
		expander = gtk_expander_new (tmp_str);
		g_free (tmp_str);
		gtk_expander_set_use_markup (GTK_EXPANDER(expander), TRUE);

		label = gtk_label_new (NULL);
		gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);
		gtk_label_set_markup (GTK_LABEL(label), _(test->instructions));
		utils_widget_set_padding (GTK_WIDGET (label), MARGIN_START, MARGIN_BOX);
		gtk_container_add (GTK_CONTAINER(expander), label);
		gtk_box_pack_start (GTK_BOX(vbox), expander, FALSE, FALSE, MARGIN_BOX);
    }

    if (test->fix)
    {
		button = gtk_button_new_with_label (_("Try to fix this inconsistency."));
		gtk_box_pack_start (GTK_BOX(vbox), button, FALSE, FALSE, 0);
		g_signal_connect_swapped (G_OBJECT(button),
								  "clicked",
								  G_CALLBACK (gsb_debug_try_fix),
								  (gpointer) test->fix);
    }

    gtk_widget_show_all (scrolled_window);

    gsb_assistant_add_page (assistant, scrolled_window, page, page - 1, -1, NULL);
    gsb_assistant_set_next (assistant, page - 1, page);
    gsb_assistant_change_button_next (assistant, "gtk-go-forward", GTK_RESPONSE_YES);
}

/**
 * reconciliation_check.
 * Cette fonction est appelée après la création de toutes les listes.
 * Elle permet de vérifier la cohérence des rapprochements suite à la
 * découverte des bogues #466 et #488.
 *
 *
 * \param
 *
 * \return a newly allocated string or NULL.
 **/
static gchar *gsb_debug_reconcile_test (void)
{
    GSList *pUserAccountsList = NULL;
    gchar *pText;
	gchar *tmp_real_str1;
	gchar *tmp_real_str2;
	gchar *tmp_str1;
	gchar *tmp_str2;
    gint affected_accounts = 0;

    /* S'il n'y a pas de compte, on quitte */
    if (!gsb_data_account_get_number_of_accounts ())
    {
		return NULL;
    }

	pText = g_strdup("");

    /* On fera la vérification des comptes dans l'ordre préféré
       de l'utilisateur. On fait une copie de la liste. */
    pUserAccountsList = g_slist_copy (gsb_data_account_get_list_accounts ());

    /* Pour chacun des comptes, faire */
    do
    {
        gpointer p_account;
        gint account_nb;
        gint reconcile_number;

        p_account = pUserAccountsList->data;
        account_nb = gsb_data_account_get_no_account (p_account);

		/* Si le compte a été rapproché au moins une fois.
        * Seule la date permet de l'affirmer. */
        reconcile_number = gsb_data_reconcile_get_account_last_number (account_nb);
        if (reconcile_number)
        {
			GSList *pTransactionList;
			GsbReal reconcilied_amount = null_real;

			/* On va recalculer le montant rapproché du compte (c-à-d le solde initial
			 * plus le montant des opérations rapprochées) et le comparer à la valeur
			 * stockée dans le fichier. Si les valeurs diffèrent, on affiche une boite
			 * d'avertissement */

			reconcilied_amount = gsb_data_account_get_init_balance (account_nb, -1);

			/* On récupère la liste des opérations */
			pTransactionList = gsb_data_transaction_get_complete_transactions_list ();

			while (pTransactionList)
			{
				gint transaction = gsb_data_transaction_get_transaction_number (pTransactionList->data);

				/* On ne prend en compte que les opérations rapprochées.
				 * On ne prend pas en compte les sous-opérations ventilées.
				 * modification aportée pour tenir compte de la transformation ultérieure
				 d'une opération simple en opération ventilée et pour avoir une correspondance
				 * entre le relevé et l'edition de déboggage */
				if (gsb_data_transaction_get_account_number (transaction) == account_nb
					&& (gsb_data_transaction_get_marked_transaction (transaction) == OPERATION_RAPPROCHEE)
					&& !gsb_data_transaction_get_mother_transaction_number (transaction))
				{
					gint currency_number;
					GsbReal number;

					currency_number = gsb_data_account_get_currency (account_nb);
					number = gsb_data_transaction_get_adjusted_amount_for_currency (transaction,
																					currency_number,
																					-1);
					reconcilied_amount = gsb_real_add (reconcilied_amount, number);
				}
				pTransactionList = pTransactionList->next;
			}

			if (gsb_real_abs (gsb_real_sub (reconcilied_amount,
											gsb_data_reconcile_get_final_balance
											(reconcile_number))).mantissa > 0)
			{
				affected_accounts ++;

				tmp_real_str1 = utils_real_get_string_with_currency (gsb_data_reconcile_get_final_balance
																	 (reconcile_number),
																	 gsb_data_account_get_currency (account_nb),
																	 TRUE);
				tmp_real_str2 = utils_real_get_string_with_currency (reconcilied_amount,
																	 gsb_data_account_get_currency (account_nb),
																	 TRUE);
				tmp_str1 = g_strdup_printf (_("<span weight=\"bold\">%s</span>\n"
											  "  Last reconciliation amount: %s\n"
											  "  Computed reconciliation amount: %s\n"),
											gsb_data_account_get_name (account_nb),
											tmp_real_str1,
											tmp_real_str2);
				tmp_str2 = pText;
				pText = g_strconcat (tmp_str2, tmp_str1, NULL);
				g_free (tmp_str2);
				g_free (tmp_str1);
				g_free (tmp_real_str1);
				g_free (tmp_real_str2);
			}
        }
    }
    while (( pUserAccountsList = pUserAccountsList->next));

    g_slist_free (pUserAccountsList);

    if (affected_accounts)
    {
		pText [ strlen(pText) - 1 ] = '\0';
		return pText;
    }

    return NULL;
}

/**
 * contra_transaction_check.
 * Cette fonction est appelée après la création de toutes les listes.
 * Elle permet de vérifier la cohérence des virements entre comptes
 * suite à la découverte du bogue #542
 *
 * \param
 *
 * \return a newly allocated string or NULL
 **/
static gchar *gsb_debug_transfer_test (void)
{
    gboolean corrupted_file = FALSE;
    GSList *pUserAccountsList;
    gchar *pText = g_strdup("");
	gchar *tmp_str;
	gchar *old_str;

    pUserAccountsList = gsb_data_account_get_list_accounts ();

    do
    {
		gboolean corrupted_account = FALSE;
		GSList *pTransactionList;
		gpointer p_account;
		gint account_nb;

        p_account = pUserAccountsList->data;
        account_nb = gsb_data_account_get_no_account (p_account);

		pTransactionList = gsb_data_transaction_get_transactions_list ();
		while (pTransactionList)
		{
			gint transaction;
			gint transfer_transaction;

			transaction = gsb_data_transaction_get_transaction_number (pTransactionList->data);
			transfer_transaction = gsb_data_transaction_get_contra_transaction_number (transaction);

			/* Si l'opération est un virement vers un compte non supprimé */
			if (gsb_data_transaction_get_account_number (transaction) == account_nb
				&& transfer_transaction > 0)
			{
				if (gsb_data_transaction_get_account_number (transfer_transaction) !=
					gsb_data_transaction_get_contra_transaction_account (transaction))
				{
					/* S'il n'y avait pas eu encore d'erreur dans ce compte,
					   on affiche son nom */
					if (!corrupted_account)
					{
						tmp_str = g_strdup_printf ("\n<span weight=\"bold\">%s</span>\n",
												   gsb_data_account_get_name (account_nb));
						old_str = pText;
						pText = g_strconcat (pText, tmp_str, NULL);
						g_free (old_str);
						g_free (tmp_str);
					}
					tmp_str = g_strdup_printf (_("Transaction %d is linked to transaction %d "
												 "which does not exist.\n"),
											   transaction,
											   transfer_transaction);
					old_str = pText;
					pText = g_strconcat (pText , tmp_str, NULL);
					g_free (old_str);
					g_free (tmp_str);
					corrupted_file = corrupted_account = TRUE;
				}
				else
				{
					if (gsb_data_transaction_get_contra_transaction_number (transfer_transaction) != transaction)
					{
						/* S'il n'y avait pas eu encore d'erreur dans ce compte,
						   on affiche son nom */
						if (!corrupted_account)
						{
							old_str = pText;
							tmp_str = g_strdup_printf ("\n<span weight=\"bold\">%s</span>\n",
													   gsb_data_account_get_name (account_nb));
							pText = g_strconcat (pText, tmp_str , NULL);
							g_free (old_str);
							g_free (tmp_str);
						}
						old_str = pText;
						tmp_str = g_strdup_printf (_("Transaction %d is linked to transaction %d "
													 "which is linked to transaction %d.\n"),
												   transaction,
												   transfer_transaction,
												   gsb_data_transaction_get_contra_transaction_number
												   (transfer_transaction));
						pText = g_strconcat (pText , tmp_str , NULL);
						g_free (old_str);
						g_free (tmp_str);
						corrupted_file = corrupted_account = TRUE;
					}
				}
			}
			pTransactionList = pTransactionList->next;
		}
		pUserAccountsList = pUserAccountsList->next;
    }
    while (pUserAccountsList);

    if (corrupted_file)
    {
		/* Skip both last and first carriage return. */
		pText [ strlen(pText) - 1 ] = '\0';
		//~ printf ("%s\n", pText);
		return pText + 1;
    }

    return NULL;
}

/**
 * check if all the categories into the transactions exist
 *
 * \param
 *
 * \return a gchar containing the transactions with problem or NULL
 **/
static gchar *gsb_debug_category_test  (void)
{
    GSList *tmp_list;
    gchar *returned_text;
    gchar *tmp_str;
    gchar *tmp_str1;
    gboolean invalid = FALSE;

	returned_text = g_strdup ("");	/* !!!don't set here my_strdup else returned_text becomes NULL */

	tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
		gint transaction_number;
		gint category_number;

		transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
		category_number = gsb_data_transaction_get_category_number (transaction_number);
		if (gsb_data_category_get_structure (category_number))
		{
			gint sub_category_number;

			/* category found, check sub-category */
			sub_category_number = gsb_data_transaction_get_sub_category_number (transaction_number);
			if (sub_category_number
				&& !gsb_data_category_get_sub_category_structure (category_number, sub_category_number))
			{
				/* sub-category not found */
				tmp_str = g_strdup_printf (_("Transaction %d has category %d and "
											 "sub-category %d which does not exist.\n"),
										   transaction_number,
										   category_number,
										   sub_category_number);
				tmp_str1 = g_strconcat (returned_text, tmp_str, NULL);
				g_free (returned_text);
				g_free (tmp_str);
				returned_text = tmp_str1;
				invalid = TRUE;
			}
		}
		else
		{
			/* category not found */
			tmp_str = g_strdup_printf (_("Transaction %d has category %d which does not exist.\n"),
									   transaction_number,
									   category_number);
			tmp_str1 = g_strconcat (returned_text, tmp_str, NULL);
			g_free (returned_text);
			g_free (tmp_str);
			returned_text = tmp_str1;
			invalid = TRUE;
		}
		tmp_list = tmp_list->next;
    }

    if (invalid)
		return returned_text;
    else
    {
		g_free (returned_text);
		return NULL;
    }
}

/**
 * fix the transactions with non-existent categories,
 * just remove the categories
 *
 * \param
 *
 * \return TRUE if ok
 **/
static gboolean gsb_debug_category_test_fix (void)
{
    GSList *tmp_list;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
		gint transaction_number;
		gint category_number;

		transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
		category_number = gsb_data_transaction_get_category_number (transaction_number);
		if (gsb_data_category_get_structure (category_number))
		{
			gint sub_category_number;

			/* category found, check sub-category */
			sub_category_number = gsb_data_transaction_get_sub_category_number (transaction_number);
			if (sub_category_number
				&& !gsb_data_category_get_sub_category_structure (category_number, sub_category_number))
				/* sub-category not found */
				gsb_data_transaction_set_sub_category_number (transaction_number, 0);
		}
		else
		{
			/* category not found */
			gsb_data_transaction_set_category_number (transaction_number, 0);
			gsb_data_transaction_set_sub_category_number (transaction_number, 0);
		}
		tmp_list = tmp_list->next;
    }

    return TRUE;
}

/**
 * check if all the budgets into the transactions exist
 *
 * \param
 *
 * \return a gchar containing the transactions with problem or NULL
 **/
static gchar *gsb_debug_budget_test  (void)
{
    GSList *tmp_list;
    gchar *returned_text;
    gchar *tmp_str;
    gchar *tmp_str1;
    gboolean invalid = FALSE;

	returned_text = g_strdup (""); 	/* !!!don't set here my_strdup else returned_text becomes NULL */

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
        gint transaction_number;
        gint budget_number;

        transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
        budget_number = gsb_data_transaction_get_budgetary_number (transaction_number);

        if (gsb_data_budget_get_structure (budget_number))
        {
            gint sub_budget_number;

            sub_budget_number = gsb_data_transaction_get_sub_budgetary_number (transaction_number);
            /* budget found, check sub-budget */
            if (sub_budget_number
             && !gsb_data_budget_get_sub_budget_structure (budget_number, sub_budget_number))
            {
            /* sub-budget not found */
                tmp_str = g_strdup_printf (_("Transaction %d has budget %d "
											 "and sub-budget %d which does not exist.\n"),
										   transaction_number,
										   budget_number,
										   sub_budget_number);
                tmp_str1 = g_strconcat (returned_text, tmp_str, NULL);
                g_free (returned_text);
                g_free (tmp_str);
                returned_text = tmp_str1;
                invalid = TRUE;
            }
        }
        else
        {
            /* budget not found */
            tmp_str = g_strdup_printf (_("Transaction %d has budget %d which does not exist.\n"),
									   transaction_number,
									   budget_number);
            tmp_str1 = g_strconcat (returned_text, tmp_str, NULL);
            g_free (returned_text);
            g_free (tmp_str);
            returned_text = tmp_str1;
            invalid = TRUE;
        }
        tmp_list = tmp_list->next;
    }

    if (invalid)
        return returned_text;
    else
    {
        g_free (returned_text);
        return NULL;
    }

    return NULL;
}

/**
 * fix the transactions with non-existent budgets,
 * just remove the categories
 *
 * \param
 *
 * \return TRUE if ok
 **/
static gboolean gsb_debug_budget_test_fix (void)
{
    GSList *tmp_list;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
        gint transaction_number;
        gint budget_number;

        transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
        budget_number = gsb_data_transaction_get_budgetary_number (transaction_number);

        if (gsb_data_budget_get_structure (budget_number))
        {
            gint sub_budget_number;

            /* budget found, check sub-budget */
            sub_budget_number = gsb_data_transaction_get_sub_budgetary_number (transaction_number);
            if (sub_budget_number
				&&
				!gsb_data_budget_get_sub_budget_structure (budget_number, sub_budget_number))
            /* sub-budget not found */
                gsb_data_transaction_set_sub_budgetary_number (transaction_number, 0);
        }
        else
        {
            /* budget not found */
            gsb_data_transaction_set_sub_budgetary_number (transaction_number, 0);
            gsb_data_transaction_set_budgetary_number (transaction_number, 0);
        }
        tmp_list = tmp_list->next;
    }
    return TRUE;
}

/**
 * check if all the payees into the transactions exist
 *
 * \param
 *
 * \return a gchar containing the transactions with problem or NULL
 **/
static gchar *gsb_debug_payee_test  (void)
{
    GSList *tmp_list;
    gchar *returned_text;
    gchar *tmp_str;
    gchar *tmp_str1;
    gboolean invalid = FALSE;

	returned_text = g_strdup (""); 	/* !!!don't set here my_strdup else returned_text becomes NULL */

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
		gint transaction_number;
		gint payee_number;

		transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
		payee_number = gsb_data_transaction_get_payee_number (transaction_number);
		if (!gsb_data_payee_get_structure (payee_number))
		{
			/* payee not found */
			tmp_str = g_strdup_printf (_("Transaction %d has payee %d which does not exist.\n"),
									   transaction_number,
									   payee_number);
			tmp_str1 = g_strconcat (returned_text, tmp_str, NULL);
			g_free (returned_text);
			g_free (tmp_str);
			returned_text = tmp_str1;
			invalid = TRUE;
		}
		tmp_list = tmp_list->next;
    }

    if (invalid)
		return returned_text;
    else
    {
		g_free (returned_text);
		return NULL;
    }
    return NULL;
}

/**
 * fix the transactions with non-existent payees,
 * just remove the categories
 *
 * \param
 *
 * \return TRUE if ok
 **/
static gboolean gsb_debug_payee_test_fix (void)
{
    GSList *tmp_list;

    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
		gint transaction_number;
		gint payee_number;

		transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
		payee_number = gsb_data_transaction_get_payee_number (transaction_number);

		if (!gsb_data_payee_get_structure (payee_number))
			gsb_data_transaction_set_payee_number (transaction_number, 0);

		tmp_list = tmp_list->next;
    }
    return TRUE;
}


/** Tests  */
static struct GsbDebugTest debug_tests [8] =
{
    /* Check for reconciliation inconcistency.  */
    { N_("Incorrect reconciliation totals"),
      N_("This test will look at accounts where the reconciled total which does not match with reconciled transactions."),
      N_("Grisbi found accounts where reconciliation totals are inconsistent with the "
        "sum of reconcilied transactions and initial balance.\n\n"
        "The cause may be the elimination or modification of reconciliations or changes "
        "in the balance of reconciliations in the preferences."),
      gsb_debug_reconcile_test, NULL },

    { N_("Duplicate sub-categories check"),
      N_("free"),
      N_("Due to a bug in previous versions of Grisbi, "
	 "sub-categories may share the same numeric identifier in some "
	 "cases, resulting in transactions having two sub-categories.  "
	 "If you choose to continue, Grisbi will "
	 "remove one of each duplicates and "
	 "recreate it with a new identifier.\n\n"
	 "No transaction will be lost, but in some cases, you "
	 "will have to manually move transactions to this new "
	 "sub-category."),
      gsb_debug_duplicate_categ_check, gsb_debug_duplicate_categ_fix },

    { N_("Duplicate sub-budgetary lines check"),
      N_("free"),
      N_("Due to a bug in previous versions of Grisbi, "
	 "sub-budgetary lines may share the same numeric id in some "
	 "cases, resulting in transactions having two sub-budgetary lines.  "
	 "If you choose to continue, Grisbi will "
	 "remove one of each duplicates and "
	 "recreate it with a new id.entifier\n\n"
	 "No transactions will be lost, but in some cases, you "
	 "will have to manually move transactions to this new "
	 "sub-budgetary line."),
      gsb_debug_duplicate_budget_check, gsb_debug_duplicate_budget_fix },

    { N_("Orphan countra-transactions check"),
      N_("free"),
      N_("In some rare cases, transfers are incorrectly linked to contra-transactions.  "
	 "This might be because of bugs or because of imports that failed.\n"
	 "To fix this, you will have to manually edit your .gsb file "
	 "(with a text editor) and fix transactions using their numeric ID."),
      gsb_debug_transfer_test, NULL },

    { N_("Incorrect category/sub-category number"),
      N_("This test will look for transactions which have non-existent categories/sub-categories."),
      N_("Grisbi found some transactions with non existents categories/sub-categories "
	 "If you choose to continue, Grisbi will remove that category error "
	 "and that transactions will have no categories."),
      gsb_debug_category_test, gsb_debug_category_test_fix },

    { N_("Incorrect budget/sub-budget number"),
      N_("This test will look for transactions which have non-existent budgets/sub-budgets."),
      N_("Grisbi found some transactions with non existents budgets/sub-budgets "
	 "If you choose to continue, Grisbi will remove that budget error "
	 "and that transactions will have no budgets."),
      gsb_debug_budget_test, gsb_debug_budget_test_fix },

    { N_("Incorrect payee number"),
      N_("This test will look for transactions which have non-existent payees."),
      N_("Grisbi found some transactions with non existents payees "
	 "If you choose to continue, Grisbi will "
	 "remove them and that transactions will have no payee."),
      gsb_debug_payee_test, gsb_debug_payee_test_fix },


    { NULL, NULL, NULL, NULL, NULL },
};

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean gsb_debug_enter_test_page ( GtkWidget *assistant)
{
    GtkTextBuffer *text_buffer = NULL;
    GtkTextIter text_iter;
    gboolean inconsistency = FALSE;
    gint i, page = 2;
	gchar *tmp_str;

    text_buffer = g_object_get_data ( G_OBJECT(assistant), "text-buffer");

    while ( gtk_notebook_get_n_pages ( g_object_get_data ( G_OBJECT (assistant),
							   "notebook")) > 2)
    {
	gtk_notebook_remove_page ( g_object_get_data ( G_OBJECT (assistant), "notebook"),
				   -1);
    }

    gtk_text_buffer_set_text ( text_buffer, "\n", -1);
    gtk_text_buffer_get_iter_at_offset ( text_buffer, &text_iter, 1);

    for ( i = 0 ; debug_tests [i] . name != NULL ; i ++)
    {
	gchar *result = debug_tests [ i ] . test ();

	if ( result)
	{
	    if ( !inconsistency)
	    {
		/* No inconsistency found yet so put title. */
		gtk_text_buffer_insert_with_tags_by_name ( text_buffer, &text_iter,
							   _("Inconsistencies found\n\n"),
							   -1, "x-large", NULL);
		gtk_text_buffer_insert ( text_buffer, &text_iter,
					 _("The following debug tests found inconsistencies "
					   "in this accounts file:\n\n"),
					 -1);
	    }

	    tmp_str = g_strconcat ( "• ", _( debug_tests[i] . name), "\n", NULL);
	    gtk_text_buffer_insert_with_tags_by_name ( text_buffer, &text_iter,
						       tmp_str,
						       -1, "indented", NULL);
            g_free ( tmp_str);

	    inconsistency = TRUE;
	    gsb_debug_add_report_page ( assistant, page, &(debug_tests[i]), result);
	    page ++;
	}
    }

    grisbi_win_status_bar_message ( _("Done"));

    if ( !inconsistency)
    {
	gtk_text_buffer_insert_with_tags_by_name ( text_buffer, &text_iter,
						   _("No inconsistency found\n\n"),
						   -1, "x-large", NULL);
	gtk_text_buffer_insert ( text_buffer, &text_iter,
				 _("Congratulations, your account file is in good shape!\n"),
				 -1);
    }

    return TRUE;
}

/******************************************************************************/
/* Public Methods                                                             */
/******************************************************************************/
/**
 * Performs various checks on Grisbi files.
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_debug (void)
{
    GtkWidget *assistant;
    GtkWidget *scrolled_window;
    GtkWidget *text_view;
    GtkTextBuffer *text_buffer;

    grisbi_win_status_bar_message (_("Checking file for possible corruption..."));

    assistant = gsb_assistant_new (_("Grisbi accounts debug"),
								   _("This assistant will help you to search your account "
									 "file for inconsistencies, which can be caused either "
									 "by bugs or by erroneous manipulation."),
								   "gsb-bug-32.png",
								   NULL);

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);

    text_view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable (GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW(text_view), FALSE);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW(text_view), MARGIN_START);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW(text_view), MARGIN_END);
    gtk_container_add (GTK_CONTAINER (scrolled_window), text_view);

    text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
    g_object_set_data (G_OBJECT (assistant), "text-buffer", text_buffer);
    gtk_text_buffer_create_tag (text_buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag (text_buffer, "x-large", "scale", PANGO_SCALE_X_LARGE, NULL);
    gtk_text_buffer_create_tag (text_buffer, "indented", "left-margin", 24, NULL);

    gsb_assistant_add_page (assistant,
							scrolled_window,
							1,
							0,
							-1,
							G_CALLBACK (gsb_debug_enter_test_page));

    gsb_assistant_run (assistant);
    gtk_widget_destroy (assistant);

    return FALSE;
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
