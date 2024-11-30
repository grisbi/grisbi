/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-2023 Pierre Biava (grisbi@pierre.biava.name)                 */
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


#include "config.h"

#include "include.h"
#include <string.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "etats_calculs.h"
#include "etats_affiche.h"
#include "etats_config.h"
#include "grisbi_app.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report.h"
#include "gsb_data_report_text_comparison.h"
#include "gsb_data_transaction.h"
#include "navigation.h"
#include "gsb_real.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint dernier_chq;		/* quand on a choisi le plus grand, contient le dernier no de chq dans les comptes choisis */
static gint dernier_pc;			/* quand on a choisi le plus grand, contient le dernier no de pc dans les comptes choisis */
static gint dernier_no_rappr;	/* quand on a choisi le plus grand, contient le dernier no de rappr dans les comptes choisis */

struct EtatAffichage * etat_affichage_output;
/*END_STATIC*/

/*START_EXTERN*/
extern struct EtatAffichage gtktable_affichage;
extern gint ligne_debut_partie;
extern gint nb_colonnes;
extern gint nb_lignes;
/*END_EXTERN*/

/* VARIABLES_À_TRAITER */
gint ancienne_ib_etat;
gint ancienne_sous_ib_etat;
gint ancienne_categ_etat;
gint ancienne_categ_speciale_etat;
gint ancienne_sous_categ_etat;
gint ancien_tiers_etat;
gint ancien_compte_etat;
GsbReal montant_categ_etat;
GsbReal montant_sous_categ_etat;
GsbReal montant_ib_etat;
GsbReal montant_sous_ib_etat;
GsbReal montant_compte_etat;
GsbReal montant_tiers_etat;
GsbReal montant_periode_etat;
GsbReal montant_exo_etat;
gint nb_ope_categ_etat;
gint nb_ope_sous_categ_etat;
gint nb_ope_ib_etat;
gint nb_ope_sous_ib_etat;
gint nb_ope_compte_etat;
gint nb_ope_tiers_etat;
gint nb_ope_periode_etat;
gint nb_ope_exo_etat;
gint nb_ope_general_etat;
gint nb_ope_partie_etat;
GDate *date_debut_periode;
gint exo_en_cours_etat;
gint changement_de_groupe_etat;
gint debut_affichage_etat;
gint devise_compte_en_cours_etat;
gint devise_categ_etat;
gint devise_ib_etat;
gint devise_tiers_etat;
gint devise_generale_etat;

const gchar *nom_categ_en_cours;
const gchar *nom_ss_categ_en_cours;
const gchar *nom_ib_en_cours;
const gchar *nom_ss_ib_en_cours;
const gchar *nom_compte_en_cours;
const gchar *nom_tiers_en_cours;
/* END_VARIABLES_À_TRAITER */

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Fonction d'alerte si le rapport est trop gros et qu'il y a un risque de plantage
 * 2 cas : Windows crash à partir de 3000 opérations, Linux et OSX à partir de ~13000
 *
 * \param guint		nbre maxi d'opérations pour alerte
 *
 * \return 			Continue ou retour vers paramètres (default)
 **/
static gint etats_dialog_warning_report_too_big (gint report_number,
												 guint nbre_opes,
												 guint nbre_max_opes)
{
	GtkWidget *dialog;
	gchar *message = NULL;
	gint result;

	message = g_strdup_printf (_("The number of transactions selected by the report is very "
								 "important (%d) and > to %d.\n"
								 "This can cause delays or a crash of Grisbi.\n"
								 "At this point you can continue or return to setting"),
							   nbre_opes,
							   nbre_max_opes);

	dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_WARNING,
									 GTK_BUTTONS_NONE,
									 NULL);

	gtk_dialog_add_buttons (GTK_DIALOG(dialog),
							_("Continue"), GTK_RESPONSE_OK,
							_("Return to settings"), GTK_RESPONSE_REJECT,
							NULL);

	gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), message);
	gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_REJECT);

	g_free (message);

	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

	result = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);

	return result;
}

/**
 * cette fonction est appelée quand l'opé a été classé dans sa categ, ib, compte ou tiers
 * et qu'elle doit être affichée ; on classe en fonction de la demande de la conf (date, no, tiers ...)
 * si les 2 opés sont équivalentes à ce niveau, on classe par no d'opé
 *
 * \param
 * \param
 *
 * \return
 **/
static gint classement_ope_perso_etat (gpointer transaction_1,
									   gpointer transaction_2)
{
	gint return_value;
	gint current_report_number;
	gint transaction_number_1;
	gint transaction_number_2;

	transaction_number_1 = gsb_data_transaction_get_transaction_number (transaction_1);
	transaction_number_2 = gsb_data_transaction_get_transaction_number (transaction_2);

	current_report_number = gsb_gui_navigation_get_current_report ();

	switch (gsb_data_report_get_sorting_report (current_report_number))
	{
		case 0:
			/* date  */
			return_value = g_date_compare (gsb_data_transaction_get_date (transaction_number_1),
										   gsb_data_transaction_get_date (transaction_number_2));
			break;

		case 1:
			/* value date  */
			if (gsb_data_transaction_get_value_date (transaction_number_1))
			{
				if (gsb_data_transaction_get_value_date (transaction_number_2))
					return_value = g_date_compare (gsb_data_transaction_get_value_date (transaction_number_1),
												   gsb_data_transaction_get_value_date (transaction_number_2));
				else
					return_value = -1;
			}
			else
			{
				if (gsb_data_transaction_get_value_date (transaction_number_2))
					return_value = 1;
				else
					return_value = g_date_compare (gsb_data_transaction_get_date (transaction_number_1),
												   gsb_data_transaction_get_date (transaction_number_2));
			}

			break;
		case 2:
			/* no opé  */
			return_value = transaction_number_1 - transaction_number_2;
			break;

		case 3:
			/* tiers  */
			if (!gsb_data_transaction_get_payee_number (transaction_number_1)
				|| !gsb_data_transaction_get_payee_number (transaction_number_2))
				return_value = gsb_data_transaction_get_payee_number (transaction_number_2) -
								gsb_data_transaction_get_payee_number (transaction_number_1);
			else
				return_value = my_strcasecmp (gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
																	   (transaction_number_1), TRUE),
											  gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
																	   (transaction_number_2), TRUE));
			break;

		case 4:
			/* categ  */
			if (!gsb_data_transaction_get_category_number (transaction_number_1)
				|| !gsb_data_transaction_get_category_number (transaction_number_2))
				return_value = gsb_data_transaction_get_category_number (transaction_number_2) -
								gsb_data_transaction_get_category_number (transaction_number_1);
			else
			{
				if (gsb_data_transaction_get_category_number (transaction_number_1) ==
					gsb_data_transaction_get_category_number (transaction_number_2)
					&&
					(!gsb_data_transaction_get_sub_category_number (transaction_number_1)
					 || !gsb_data_transaction_get_sub_category_number (transaction_number_2)))
					return_value = gsb_data_transaction_get_sub_category_number (transaction_number_2)-
										gsb_data_transaction_get_sub_category_number (transaction_number_1);
			else
			{
				gchar *category_name_1 = gsb_data_category_get_name (gsb_data_transaction_get_category_number
															  (transaction_number_1),
																	   gsb_data_transaction_get_sub_category_number
																	   (transaction_number_1),
																	   NULL);
				gchar *category_name_2 = gsb_data_category_get_name (gsb_data_transaction_get_category_number
																	 (transaction_number_2),
																		gsb_data_transaction_get_sub_category_number
																		(transaction_number_2),
																		NULL);
				return_value = my_strcasecmp (category_name_1, category_name_2);
				g_free(category_name_1);
				g_free(category_name_2);
			}
			}
			break;

		case 5:
			/* ib  */
			if (!gsb_data_transaction_get_budgetary_number (transaction_number_1)
				|| !gsb_data_transaction_get_budgetary_number (transaction_number_2))
				return_value = gsb_data_transaction_get_budgetary_number (transaction_number_2) -
								gsb_data_transaction_get_budgetary_number (transaction_number_1);
			else
			{
				if (gsb_data_transaction_get_budgetary_number (transaction_number_1) ==
					gsb_data_transaction_get_budgetary_number (transaction_number_2)
					&& (!gsb_data_transaction_get_sub_budgetary_number (transaction_number_1)
						|| !gsb_data_transaction_get_sub_budgetary_number (transaction_number_2)))
					return_value = gsb_data_transaction_get_sub_budgetary_number (transaction_number_2) -
										gsb_data_transaction_get_sub_budgetary_number (transaction_number_1);
			else
				return_value = my_strcasecmp (gsb_data_budget_get_name (gsb_data_transaction_get_budgetary_number
																		(transaction_number_1),
																		gsb_data_transaction_get_sub_budgetary_number
																		(transaction_number_1),
																		NULL),
											  gsb_data_budget_get_name (gsb_data_transaction_get_budgetary_number
																		(transaction_number_2),
																		gsb_data_transaction_get_sub_budgetary_number
																		(transaction_number_2),
																		NULL));
			}
			break;

		case 6:
			/* note si une des 2 opés n'a pas de notes, elle va en 2ème */
			if (gsb_data_transaction_get_notes (transaction_number_1)
				&& gsb_data_transaction_get_notes (transaction_number_2))
			return_value = my_strcasecmp (gsb_data_transaction_get_notes (transaction_number_1),
										  gsb_data_transaction_get_notes (transaction_number_2));
			else
				return_value = GPOINTER_TO_INT (gsb_data_transaction_get_notes (transaction_number_2)) -
								GPOINTER_TO_INT (gsb_data_transaction_get_notes (transaction_number_1));
			break;

		case 7:
			/* type ope  */
			if (!gsb_data_transaction_get_method_of_payment_number (transaction_number_1)
				|| !gsb_data_transaction_get_method_of_payment_number (transaction_number_2))
				return_value = gsb_data_transaction_get_method_of_payment_number (transaction_number_2) -
								gsb_data_transaction_get_method_of_payment_number (transaction_number_1);
			else
			{
				/* les opés peuvent provenir de 2 comptes différents, il faut donc trouver les 2 types dans les */
				/* listes différentes */
				return_value = my_strcasecmp (gsb_data_payment_get_name (gsb_data_transaction_get_method_of_payment_number
																		 (transaction_number_1)),
											  gsb_data_payment_get_name (gsb_data_transaction_get_method_of_payment_number
																		 (transaction_number_2)));
			}
			break;

		case 8:
			/* no chq  */
			if (gsb_data_transaction_get_method_of_payment_content (transaction_number_1)
				&& gsb_data_transaction_get_method_of_payment_content (transaction_number_2))
			return_value = my_strcasecmp (gsb_data_transaction_get_method_of_payment_content (transaction_number_1),
										  gsb_data_transaction_get_method_of_payment_content (transaction_number_2));
			else
				return_value = GPOINTER_TO_INT (gsb_data_transaction_get_method_of_payment_content
												(transaction_number_2)) -
								GPOINTER_TO_INT (gsb_data_transaction_get_method_of_payment_content
												 (transaction_number_1));
			break;

		case 9:
			/* pc  */
			if (gsb_data_transaction_get_voucher (transaction_number_1)
				&& gsb_data_transaction_get_voucher (transaction_number_2))
				return_value = my_strcasecmp (gsb_data_transaction_get_voucher (transaction_number_1),
											  gsb_data_transaction_get_voucher (transaction_number_2));
			else
				return_value = GPOINTER_TO_INT (gsb_data_transaction_get_voucher (transaction_number_2)) -
								GPOINTER_TO_INT (gsb_data_transaction_get_voucher (transaction_number_1));
			break;

		case 10:
			/* ibg  */
			if (gsb_data_transaction_get_bank_references (transaction_number_1)
				&& gsb_data_transaction_get_bank_references (transaction_number_2))
				return_value = my_strcasecmp (gsb_data_transaction_get_bank_references (transaction_number_1),
											  gsb_data_transaction_get_bank_references (transaction_number_2));
			else
				return_value = GPOINTER_TO_INT (gsb_data_transaction_get_bank_references (transaction_number_2)) -
								GPOINTER_TO_INT (gsb_data_transaction_get_bank_references (transaction_number_1));
			break;

		case 11:
			/* no rappr  */
			if (!gsb_data_transaction_get_reconcile_number (transaction_number_1)
				||
				!gsb_data_transaction_get_reconcile_number (transaction_number_2))
				return_value = gsb_data_transaction_get_reconcile_number (transaction_number_2) -
								gsb_data_transaction_get_reconcile_number (transaction_number_1);
			else
				return_value = my_strcasecmp (gsb_data_reconcile_get_name(gsb_data_transaction_get_reconcile_number
																		  (transaction_number_1)),
											  gsb_data_reconcile_get_name (gsb_data_transaction_get_reconcile_number
																		   (transaction_number_2)));
			break;

		default :
			return_value = 0;
	}


	if (!return_value)
		return_value = transaction_number_1 - transaction_number_2;

	return (return_value);
}

/**
 * Fonction de classement de la liste en fonction du choix du type de classement
 *
 * \param
 * \param
 *
 * \return
 **/
static gint classement_liste_opes_etat (gpointer transaction_1,
										gpointer transaction_2)
{
	GSList *pointeur;
	gint current_report_number;
	gint return_value;
	gint transaction_number_1;
	gint transaction_number_2;

	transaction_number_1 = gsb_data_transaction_get_transaction_number (transaction_1);
	transaction_number_2 = gsb_data_transaction_get_transaction_number (transaction_2);

	current_report_number = gsb_gui_navigation_get_current_report ();

	pointeur = gsb_data_report_get_sorting_type_list (current_report_number);

classement_suivant:

	/* si pointeur est null, on a fait le tour du classement, les opés sont identiques */
	/* si elles sont affichées, on classe suivant classement demandé puis par no d'opé si identiques */
	/* sinon on repart en mettant -1 */

	if (!pointeur)
	{
		return (classement_ope_perso_etat (transaction_1, transaction_2));
	}

	switch (GPOINTER_TO_INT (pointeur->data))
	{
		/* classement des catégories */
		case 1:
			/* aide mémoire...
			 * principe de fonctionnement :
			 * en haut les categs,
			 * en dessous les sans categs,
			 * en dessous les ventils
			 * en dessous les virements
			 *
			 * pour chaque possibilité de l'opé 1, on va tester chaque possibilité de l'opé 2
			 * cad on va tester l'opé 1 sur categ, pas de categ, ventil, virement
			 * et pour chacun on teste l'opé 2 sur categ, pas de categ, ventil, virement
			 * */
			if (gsb_data_report_get_category_used (current_report_number))
			{
				gint category_number_1;
				gint category_number_2;

				category_number_1 = gsb_data_transaction_get_category_number (transaction_number_1);
				category_number_2 = gsb_data_transaction_get_category_number (transaction_number_2);

				/* Si les catégories sont nulles, on doit départager
				 * entre virements, pas de categ ou opé ventilée on
				 * met en 1er les opés sans categ, ensuite les
				 * ventilations et enfin les virements */
				if (category_number_1)
				{
					if (category_number_2)
					{
						/* 2 categories, return sorted */
						gchar *category_name_1;
						gchar *category_name_2;

						if (category_number_1 == category_number_2)
						{
							/* the 2 categories are the same, go to sub categ */
							pointeur = pointeur->next;
							goto classement_suivant;
						}

						category_name_1 = gsb_data_category_get_name (category_number_1, 0, NULL);
						category_name_2 = gsb_data_category_get_name (category_number_2, 0, NULL);
						return_value = my_strcasecmp (category_name_1, category_name_2);
						g_free(category_name_1);
						g_free(category_name_2);

						if (return_value)
						{
							return return_value;
						}
						else
						{
							/* the categories have the same name, go to sub categ */
							pointeur = pointeur->next;
							goto classement_suivant;
						}
					}
					/* here, categ 2 is 0 and categ 1 is not 0, so categ 1 goes before categ 2 */
					return -1;
				}

				/* come here if transaction_1 has no category, so can be transfer or split */
				if (gsb_data_transaction_get_split_of_transaction (transaction_number_1))
				{
					if (category_number_2)
						/* transaction_2 has a category => go before transaction_1 */
						return 1;
					if (gsb_data_transaction_get_contra_transaction_number (transaction_number_2) > 0)
						/* transaction_2 is a transfer, and transaction_1 a split, so split first please */
						return -1;
					if (gsb_data_transaction_get_split_of_transaction (transaction_number_2))
					{
						/* ok, the 2 transactions are split, so cannot separate them with category and
						 * sub category, so go to 2 times after that sort */
						pointeur = pointeur->next;
						pointeur = pointeur->next;
						goto classement_suivant;
					}
					/* category_number_2 is 0, so go go before the transaction_1 (without categ come before split or transfer) */
					return 1;
				}

				/* come here if transaction_1 has no category and is not a split */
				if (gsb_data_transaction_get_contra_transaction_number (transaction_number_1) > 0)
				{
					if (category_number_2)
						/* transaction_2 has a category => go before transaction_1 */
						return 1;
					if (gsb_data_transaction_get_split_of_transaction (transaction_number_2))
						/* transaction_2 is a split, and transaction_1 a transfer, so split first please */
						return 1;
					if (gsb_data_transaction_get_contra_transaction_number (transaction_number_2) > 0)
					{
						/* the 2 transactions are transfer, we return a strcmp on their name */
						return_value = my_strcasecmp (gsb_data_account_get_name
													  (gsb_data_transaction_get_contra_transaction_number
													   (transaction_number_1)),
													  gsb_data_account_get_name
													  (gsb_data_transaction_get_contra_transaction_number
													   (transaction_number_2)));

						if (return_value)
							return return_value;
						else
						{
							/* transfer to the same name of accout, go to the next sort after the sub categ */
							pointeur = pointeur->next;
							pointeur = pointeur->next;
							goto classement_suivant;
						}
					}
					/* transaction_2 has no categ, so go before transaction_1 */
					return 1;
				}

				/* come here if transaction_1 has no categ, no transfer and no split,
				 * return according to transaction_2 */
				if (category_number_2)
					return 1;
				if (gsb_data_transaction_get_split_of_transaction (transaction_number_2)
					|| gsb_data_transaction_get_contra_transaction_number (transaction_number_2) > 0)
					return -1;

				/* transaction_2 has too no categ, no transfer and no split,
				 * so jumb sub categ and go to the next sort */
				pointeur = pointeur->next;
				pointeur = pointeur->next;
				goto classement_suivant;
			}
			else
			{
				/* don't use category, go to the next sort */
				pointeur = pointeur->next;
				pointeur = pointeur->next;
				goto classement_suivant;
			}
			break;

		/* classement des sous catégories */
		case 2:
			/* normaly can come here only if it's a real category, so don't check here */
			if (gsb_data_report_get_category_used (current_report_number)
				&& gsb_data_report_get_category_show_sub_category (current_report_number))
			{
				/* we sort by sub-categ, alphabetic order, and first with sub-categ, no sub-categ come after */
				gchar *categ_name_1;
				gchar *categ_name_2;

				categ_name_1 = gsb_data_category_get_name (gsb_data_transaction_get_category_number
														   (transaction_number_1),
														   gsb_data_transaction_get_sub_category_number
														   (transaction_number_1),
														   NULL);
				categ_name_2 = gsb_data_category_get_name (gsb_data_transaction_get_category_number
														   (transaction_number_2),
														   gsb_data_transaction_get_sub_category_number
														   (transaction_number_2),
														   NULL);
				return_value = my_strcasecmp (categ_name_1, categ_name_2);
				g_free(categ_name_1);
				g_free(categ_name_2);

				if (return_value)
				{
					return return_value;
				}
				else
				{
					/* the sub-categs have the same name, go to next sort */
					pointeur = pointeur->next;
					goto classement_suivant;
				}
			}
			else
			{
				/* don't use sub_categ */
				pointeur = pointeur->next;
				goto classement_suivant;
			}
			break;

		/* classement des ib */
		case 3:
			if (gsb_data_report_get_budget_used (current_report_number))
			{
				/* we sort by budget, alphabetic order, and first with budgets, no budgets come after */
				const gchar *budget_name_1;
				const gchar *budget_name_2;

				budget_name_1 = gsb_data_budget_get_name (gsb_data_transaction_get_budgetary_number
														  (transaction_number_1),
														  0,
														  NULL);
				budget_name_2 = gsb_data_budget_get_name (gsb_data_transaction_get_budgetary_number
														  (transaction_number_2),
														  0,
														  NULL);
				return_value = my_strcasecmp (budget_name_1, budget_name_2);

				if (return_value)
				{
					return return_value;
				}
				else
				{
					/* the budgets have the same name, go to sub budget */
					pointeur = pointeur->next;
					goto classement_suivant;
				}
			}
			else
			{
				/* don't use budgets */
				pointeur = pointeur->next;
				pointeur = pointeur->next;
				goto classement_suivant;
			}

			break;

		/* classement des sous ib */
		case 4:
			if (gsb_data_report_get_budget_used (current_report_number)
				&& gsb_data_report_get_budget_show_sub_budget (current_report_number))
			{
				/* we sort by sub-budget, alphabetic order, and first with sub-budgets, no sub-budgets come after */
				const gchar *budget_name_1;
				const gchar *budget_name_2;

				budget_name_1 = gsb_data_budget_get_name (gsb_data_transaction_get_budgetary_number
														  (transaction_number_1),
														  gsb_data_transaction_get_sub_budgetary_number
														  (transaction_number_1),
														  NULL);
				budget_name_2 = gsb_data_budget_get_name (gsb_data_transaction_get_budgetary_number
														  (transaction_number_2),
														  gsb_data_transaction_get_sub_budgetary_number
														  (transaction_number_2),
														  NULL);
				return_value = my_strcasecmp (budget_name_1, budget_name_2);

				if (return_value)
				{
					return return_value;
				}
				else
				{
					/* the sub-budgets have the same name, go to next sort */
					pointeur = pointeur->next;
					goto classement_suivant;
				}
			}
			else
			{
				/* don't use sub-budget */
				pointeur = pointeur->next;
				goto classement_suivant;
			}
			break;

		/* classement des comptes */
		case 5:
			if (gsb_data_report_get_account_group_reports (current_report_number))
			{
				/* sort by account, alphabetic order
				 * TODO : perhaps ask in parameters for alphabetic or different order ?*/
				gchar *account_name_1;
				gchar *account_name_2;

				account_name_1 = gsb_data_account_get_name (gsb_data_transaction_get_account_number
															(transaction_number_1));
				account_name_2 = gsb_data_account_get_name (gsb_data_transaction_get_account_number
															(transaction_number_2));
				return_value = my_strcasecmp (account_name_1, account_name_2);

				if (return_value)
				{
					return return_value;
				}
				else
				{
					/* the accounts have the same name, go to next sort */
					pointeur = pointeur->next;
					goto classement_suivant;
				}
			}
			else
			{
				/* don't use account sort, go to next sort */
				pointeur = pointeur->next;
				goto classement_suivant;
			}
			break;

			/* classement des tiers */
		case 6:
			if (gsb_data_report_get_payee_used (current_report_number))
			{
				/* sort by party, alphabetic order */
				const gchar *party_name_1;
				const gchar *party_name_2;

				party_name_1 = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
														(transaction_number_1), TRUE);
				party_name_2 = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
														(transaction_number_2), TRUE);
				return_value = my_strcasecmp (party_name_1, party_name_2);

				if (return_value)
				{
					return return_value;
				}
				else
				{
					/* the payees have the same name, go to next sort */
					pointeur = pointeur->next;
					goto classement_suivant;
				}
			}
			else
			{
				/* don't use payee sort, go to next sort */
				pointeur = pointeur->next;
				goto classement_suivant;
			}
			break;
	}
	/* not a good idea to return 0 because means equal,
	 * normaly don't come here */
	return (-1);
}

/**
 * compare les 2 no de chq en fonction du comparateur donné en argument
 *
 * \param
 * \param
 * \param
 *
 * \return	 1 si l'opé passe le test, sinon 0
 **/
static gint compare_cheques_etat (gint chq_ope,
								  gint chq_test,
								  gint comparateur)
{
	gint return_value = 0;

	switch (comparateur)
	{
		case 0:
			/* =  */
			if (chq_ope == chq_test)
				return_value = 1;
			break;

		case 1:
			/* <  */
			if (chq_ope < chq_test)
				return_value = 1;
			break;

		case 2:
			/* <=  */
			if (chq_ope <= chq_test)
				return_value = 1;
			break;

		case 3:
			/* >  */
			if (chq_ope > chq_test)
				return_value = 1;
			break;

		case 4:
			/* >=  */
			if (chq_ope >= chq_test)
				return_value = 1;
			break;

		case 5:
			/* !=  */
			if (chq_ope != chq_test)
				return_value = 1;
			break;

		case 6:
			/* le plus grand */
			if (chq_ope == chq_test)
				return_value = 1;
			break;
	}

	return (return_value);
}

/**
 * compare les 2 montants en fonction du comparateur donné en argument
 *
 * \param
 * \param
 * \param
 *
 * \return	1 si l'opé passe le test, sinon 0
 **/
static gint compare_montants_etat (GsbReal montant_ope,
								   GsbReal montant_test,
								   gint comparateur)
{
	gint return_value = 0;

	switch (comparateur)
	{
		case 0:
			/* =  */
			if (!gsb_real_cmp (montant_ope, montant_test))
				return_value = 1;
			break;

		case 1:
			/* <  */
			if (gsb_real_cmp (montant_ope, montant_test) < 0)
				return_value = 1;
			break;

		case 2:
			/* <=  */
			if (gsb_real_cmp (montant_ope, montant_test) <= 0)
			return_value = 1;
			break;

		case 3:
			/* >  */
			if (gsb_real_cmp (montant_ope, montant_test) > 0)
				return_value = 1;
			break;

		case 4:
			/* >=  */
			if (gsb_real_cmp (montant_ope, montant_test) >= 0)
				return_value = 1;
			break;

		case 5:
			/* !=  */
			if (gsb_real_cmp (montant_ope, montant_test))
				return_value = 1;
			break;

		case 6:
			/* nul  */
			if (!montant_ope.mantissa)
				return_value = 1;
			break;

		case 7:
			/* non nul  */
			if (montant_ope.mantissa)
				return_value = 1;
			break;

		case 8:
			/* positif  */
			if (montant_ope.mantissa > 0)
				return_value = 1;
			break;

		case 9:
			/* négatif  */
			if (montant_ope.mantissa < 0)
				return_value = 1;
			break;
	}
	return (return_value);
}

/**
 * récupère le texte pour faire le test sur les textes
 *
 * \param
 * \param gint		N° du champ recherché
 *
 * \return
 **/
static const gchar *recupere_texte_test_etat (gint transaction_number,
											  gint champ)
{
	const gchar *texte;

	switch (champ)
	{
		case 0:
			/* tiers  */
			texte = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number (transaction_number), TRUE);
			break;

		case 1:
			/* info du tiers */
			texte = gsb_data_payee_get_description (gsb_data_transaction_get_payee_number (transaction_number));
			break;

		case 2:
			/* categ */
			texte = gsb_data_category_get_name (gsb_data_transaction_get_category_number (transaction_number),
												0,
												NULL);
			break;

		case 3:
			/* ss-categ */
			texte = gsb_data_category_get_sub_category_name (gsb_data_transaction_get_category_number
															 (transaction_number),
															 gsb_data_transaction_get_sub_category_number
															 (transaction_number),
															 NULL);
			break;

		case 4:
			/* ib */
			texte = gsb_data_budget_get_name (gsb_data_transaction_get_budgetary_number (transaction_number),
											  0,
											  NULL);
			break;

		case 5:
			/* ss-ib */
			texte = gsb_data_budget_get_name (gsb_data_transaction_get_budgetary_number (transaction_number),
											  gsb_data_transaction_get_sub_budgetary_number (transaction_number),
											  NULL);
			break;

		case 6:
			/* notes  */
			texte = gsb_data_transaction_get_notes (transaction_number);
			break;

		case 7:
			/* ref bancaires  */
			texte = gsb_data_transaction_get_bank_references (transaction_number);
			break;

		case 8:
			/* pc */
			texte = gsb_data_transaction_get_voucher (transaction_number);
			break;

		case 9:
			/* chq  */
			texte = gsb_data_transaction_get_method_of_payment_content (transaction_number);
			break;

		case 10:
			/* no rappr */
			texte = gsb_data_reconcile_get_name (gsb_data_transaction_get_reconcile_number (transaction_number));
			break;

		default:
			texte = NULL;
	}

	return (texte);
}

/**
 * vérifie si l'opé passe le test du chq
 *
 * \param
 * \param
 *
 * \return
 **/
static gint verifie_chq_test_etat (gint text_comparison_number,
								   const gchar *no_chq)
{
	gint tmp_comparison;
	gint ope_dans_test;
	gint ope_dans_premier_test = 0;
	gint ope_dans_second_test = 0;

	tmp_comparison = gsb_data_report_text_comparison_get_first_comparison (text_comparison_number);
	/* si on cherche le plus grand, on met la valeur recherchée à la place de montant_1 */
	if (tmp_comparison != 6)
		ope_dans_premier_test = compare_cheques_etat (utils_str_atoi (no_chq),
													  gsb_data_report_text_comparison_get_first_amount
													  (text_comparison_number),
													  tmp_comparison);
	else
	{
		gint reconcile_number;

		switch (gsb_data_report_text_comparison_get_field (text_comparison_number))
		{
			case 8:
			/* pc */
			ope_dans_premier_test = compare_cheques_etat (utils_str_atoi (no_chq),
														  dernier_pc,
														  tmp_comparison);
			break;

			case 9:
			/* chq */
			ope_dans_premier_test = compare_cheques_etat (utils_str_atoi (no_chq),
														  dernier_chq,
														  tmp_comparison);
			break;

			case 10:
			/* rappr */
			/* no_chq contains the reconcile_number */
			reconcile_number = gsb_data_reconcile_get_number_by_name (no_chq);

			if (reconcile_number)
				ope_dans_premier_test = compare_cheques_etat (reconcile_number,
															  dernier_no_rappr,
															  tmp_comparison);
			break;
		}
	}

	if (gsb_data_report_text_comparison_get_link_first_to_second_part (text_comparison_number) != 3)
	{
		tmp_comparison = gsb_data_report_text_comparison_get_second_comparison (text_comparison_number);
		if (tmp_comparison != 6)
			ope_dans_second_test = compare_cheques_etat (utils_str_atoi (no_chq),
														 gsb_data_report_text_comparison_get_second_amount
														 (text_comparison_number),
														 tmp_comparison);
		else
		{
			switch (gsb_data_report_text_comparison_get_field (text_comparison_number))
			{
				case 8:
					/* pc */
					ope_dans_second_test = compare_cheques_etat (utils_str_atoi (no_chq),
																 dernier_pc,
																 tmp_comparison);
					break;

				case 9:
					/* chq */
					ope_dans_second_test = compare_cheques_etat (utils_str_atoi (no_chq),
																 dernier_chq,
																 tmp_comparison);
					break;

				case 10:
					/* rappr */
					ope_dans_second_test = compare_cheques_etat (utils_str_atoi (no_chq),
																 dernier_no_rappr,
																 tmp_comparison);
					break;
			}
		}
	}

	switch (gsb_data_report_text_comparison_get_link_first_to_second_part (text_comparison_number))
	{
		case 0:
			/* et  */
			ope_dans_test = ope_dans_premier_test && ope_dans_second_test;
			break;

		case 1:
			/*  ou */
			ope_dans_test = ope_dans_premier_test || ope_dans_second_test;
			break;

		case 2:
			/* sauf  */
			ope_dans_test = ope_dans_premier_test && (!ope_dans_second_test);
			break;

		case 3:
			/* aucun  */
			ope_dans_test = ope_dans_premier_test;
			break;

		default:
			ope_dans_test = 0;
	}

	return (ope_dans_test);
}

/**
 * vérifie si l'opé passe le test du texte
 *
 * \param
 * \param
 *
 * \return
 **/
static gint verifie_texte_test_etat (gint text_comparison_number,
									 const gchar *texte_ope)
{
	const gchar *tmp_str;
	gint ope_dans_test;

	ope_dans_test = 0;

	tmp_str = gsb_data_report_text_comparison_get_text (text_comparison_number);
	switch (gsb_data_report_text_comparison_get_operator (text_comparison_number))
	{
		case 0:
			/* contient  */
			if (texte_ope && tmp_str)
			{
				if (g_strstr_len (texte_ope, -1, tmp_str))
					ope_dans_test = 1;
			}
			break;

		case 1:
			/* ne contient pas  */
			if (texte_ope && tmp_str)
			{
				if (!g_strstr_len (texte_ope, -1, tmp_str))
					ope_dans_test = 1;
			}
			else
				ope_dans_test = 1;
			break;

		case 2:
			/* commence par  */
			if (texte_ope && tmp_str)
			{
				if (g_str_has_prefix (texte_ope, tmp_str))
					ope_dans_test = 1;
			}
			break;

		case 3:
			/* se termine par  */
			if (texte_ope && tmp_str)
			{
				if (g_str_has_suffix (texte_ope, tmp_str))
					ope_dans_test = 1;
			}
			break;

		case 4:
			/* vide  */
			if (!texte_ope)
				ope_dans_test = 1;
			break;

		case 5:
			/* non vide  */
			if (texte_ope)
				ope_dans_test = 1;
			break;
	}

	return (ope_dans_test);
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
static void etape_finale_affichage_etat (GSList *ope_selectionnees,
										 struct EtatAffichage *affichage,
										 gchar *filename)
{
	GSList *liste_ope_revenus;
	GSList *liste_ope_depenses;
	GSList *pointeur_opes;
	GSList *pointeur_sort_list;
	const gchar *decalage_base;
	const gchar *decalage_categ;
	const gchar *decalage_sous_categ;
	const gchar *decalage_ib;
	const gchar *decalage_sous_ib;
	const gchar *decalage_compte;
	const gchar *decalage_tiers;
	gint i;
	gint ligne;
	gint current_report_number;
	gint categ_used = 0;
	gint sous_categ_used = 0;
	gint group_reports = 0;
	gint ib_used = 0;
	gint sous_ib_used = 0;
	gint payee_used = 0;
	GsbReal total_general;
	GsbReal total_partie;

	current_report_number = gsb_gui_navigation_get_current_report ();

	/* initialisations variables permanentes */
	categ_used = gsb_data_report_get_category_used (current_report_number);
	sous_categ_used = gsb_data_report_get_category_show_sub_category (current_report_number);
	ib_used = gsb_data_report_get_budget_used (current_report_number);
	sous_ib_used = gsb_data_report_get_budget_show_sub_budget (current_report_number);
	group_reports = gsb_data_report_get_account_group_reports (current_report_number);
	payee_used = gsb_data_report_get_payee_used (current_report_number);

	/* soit on sépare en revenus et dépenses, soit non */
	liste_ope_revenus = NULL;
	liste_ope_depenses = NULL;
	pointeur_opes = ope_selectionnees;
	pointeur_sort_list = gsb_data_report_get_sorting_type_list (current_report_number);

	if (gsb_data_report_get_split_credit_debit (current_report_number))
	{
		/* on commence par séparer la liste revenus et de dépenses */
		/* si le classement racine est la catégorie, on sépare par catégorie de revenu ou de dépense */
		/* si c'est un autre, on sépare par montant positif ou négatif */
		while (pointeur_opes)
		{
			gint transaction_number;

			transaction_number = gsb_data_transaction_get_transaction_number (pointeur_opes->data);

			if (GPOINTER_TO_INT (pointeur_sort_list->data) == 1)
			{
				/* le classement racine est la catégorie */
				/* s'il n'y a pas de catég, c'est un virement ou une ventilation */
				/* dans ce cas, on classe en fonction du montant */
				gint category_number;

				if ((category_number = gsb_data_transaction_get_category_number (transaction_number)))
				{
					if (gsb_data_category_get_type (category_number))
						liste_ope_depenses = g_slist_append (liste_ope_depenses,
															 gsb_data_transaction_get_pointer_of_transaction
															 (transaction_number));
					else
						liste_ope_revenus = g_slist_append (liste_ope_revenus,
															gsb_data_transaction_get_pointer_of_transaction
															(transaction_number));
				}
				else
				{
					if (gsb_data_transaction_get_amount (transaction_number).mantissa < 0)
						liste_ope_depenses = g_slist_append (liste_ope_depenses,
															 gsb_data_transaction_get_pointer_of_transaction
															 (transaction_number));
					else
					liste_ope_revenus = g_slist_append (liste_ope_revenus,
														gsb_data_transaction_get_pointer_of_transaction
														(transaction_number));
				}
			}
			else
			{
				/* le classement racine n'est pas la catég, on sépare en fonction du montant */
				if (gsb_data_transaction_get_amount (transaction_number).mantissa < 0)
					liste_ope_depenses = g_slist_append (liste_ope_depenses,
														 gsb_data_transaction_get_pointer_of_transaction
														 (transaction_number));
				else
					liste_ope_revenus = g_slist_append (liste_ope_revenus,
														gsb_data_transaction_get_pointer_of_transaction
														(transaction_number));
			}
			pointeur_opes = pointeur_opes->next;
		}
	}
	else
		/* on ne veut pas séparer en revenus et dépenses, on garde juste la liste d'opé telle quelle */
		liste_ope_revenus = ope_selectionnees;

	/* on va maintenant classer ces 2 listes dans l'ordre adéquat */
	liste_ope_depenses = g_slist_sort (liste_ope_depenses, (GCompareFunc) classement_liste_opes_etat);
	liste_ope_revenus = g_slist_sort (liste_ope_revenus, (GCompareFunc) classement_liste_opes_etat);

	/* calcul du décalage pour chaque classement */
	/* c'est une chaine vide qu'on ajoute devant le nom du classement (tiers, ib ...) */
	/* on met 2 espaces par décalage */
	/* normalement, pointeur_sort_list est déjà positionné */
	decalage_base = "";

	/* pour éviter le warning lors de la compilation, on met */
	/* toutes les var char à "" */
	decalage_categ = "";
	decalage_sous_categ = "";
	decalage_ib = "";
	decalage_sous_ib = "";
	decalage_compte = "";
	decalage_tiers = "";

	while (pointeur_sort_list)
	{
		switch (GPOINTER_TO_INT (pointeur_sort_list->data))
		{
			/* décalage de la catégorie */
			case 1:
			if (gsb_data_report_get_category_used (current_report_number))
				decalage_categ = g_strconcat (decalage_base, "    ", NULL);
			else
			{
				decalage_categ = decalage_base;
				goto pas_decalage;
			}
			break;

			/* décalage de la ss-catégorie */
			case 2:
			if (gsb_data_report_get_category_used (current_report_number)
				 &&
				 gsb_data_report_get_category_show_sub_category (current_report_number))
				decalage_sous_categ = g_strconcat (decalage_base, "    ", NULL);
			else
			{
				decalage_sous_categ = decalage_base;
				goto pas_decalage;
			}
			break;

			/* décalage de l'ib */
			case 3:
			if (gsb_data_report_get_budget_used (current_report_number))
				decalage_ib = g_strconcat (decalage_base, "    ", NULL);
			else
			{
				decalage_ib = decalage_base;
				goto pas_decalage;
			}
			break;

			/* décalage de la ss-ib */
			case 4:
			if (gsb_data_report_get_budget_used (current_report_number)
				 &&
				 gsb_data_report_get_budget_show_sub_budget (current_report_number))
				decalage_sous_ib = g_strconcat (decalage_base, "    ", NULL);
			else
			{
				decalage_sous_ib = decalage_base;
				goto pas_decalage;
			}
			break;

			/* décalage du compte */
			case 5:
			if (gsb_data_report_get_account_group_reports (current_report_number))
				decalage_compte = g_strconcat (decalage_base, "    ", NULL);
			else
			{
				decalage_compte = decalage_base;
				goto pas_decalage;
			}
			break;

			/* décalage du tiers */
			case 6:
			if (gsb_data_report_get_payee_used (current_report_number))
				decalage_tiers = g_strconcat (decalage_base, "    ", NULL);
			else
			{
				decalage_tiers = decalage_base;
				goto pas_decalage;
			}
			break;
		}
		decalage_base = g_strconcat (decalage_base, "    ", NULL);

pas_decalage:
		pointeur_sort_list = pointeur_sort_list->next;
	}

	/* calcul du nb de colonnes : */
	/* 1ère pour les titres de structure la dernière pour les montants */
	/* et entre les 2 pour l'affichage des opés->variable */
	nb_colonnes = 0;

	if (gsb_data_report_get_show_report_transactions (current_report_number))
	{
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_date (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_value_date (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_account_name (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_payee (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_category (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_budget (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_note (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_voucher (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_bank_references (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_transaction_number (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_method_of_payment (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_method_of_payment_content (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_marked (current_report_number);
		nb_colonnes = nb_colonnes + gsb_data_report_get_show_report_financial_year (current_report_number);

		/* on ajoute les séparations */
		nb_colonnes = 2 * nb_colonnes - 1;
	}
	nb_colonnes = nb_colonnes + 3;
	ligne_debut_partie = -1;

	nom_categ_en_cours = NULL;
	nom_ss_categ_en_cours = NULL;
	nom_ib_en_cours = NULL;
	nom_ss_ib_en_cours = NULL;
	nom_compte_en_cours = NULL;
	nom_tiers_en_cours = NULL;

	if (!etat_affiche_initialise (ope_selectionnees, filename))
	{
		return;
	}

	/* on commence à remplir le tableau */
	/* on met le titre */
	total_general = null_real;
	nb_ope_general_etat = 0;
	ligne = etat_affiche_affiche_titre_etat (0);

	/* séparation */
	ligne = etat_affiche_affiche_separateur (ligne);

	/*   si nécessaire, on met les titres des colonnes */
	if (gsb_data_report_get_show_report_transactions (current_report_number)
		&& gsb_data_report_get_column_title_show (current_report_number)
		&& !gsb_data_report_get_column_title_type (current_report_number))
		ligne = etat_affiche_affiche_titres_colonnes (ligne);

	/* on met directement les adr des devises de categ, ib et tiers en global pour */
	/* gagner de la vitesse */
	devise_categ_etat = gsb_data_report_get_category_currency (current_report_number);
	devise_ib_etat = gsb_data_report_get_budget_currency (current_report_number);
	devise_tiers_etat = gsb_data_report_get_payee_currency (current_report_number);
	devise_generale_etat = gsb_data_report_get_currency_general (current_report_number);

	for (i=0 ; i<2 ; i++)
	{
		ancienne_categ_etat = -1;
		ancienne_categ_speciale_etat = 0;
		ancienne_sous_categ_etat = -1;
		ancienne_ib_etat = -1;
		ancienne_sous_ib_etat = -1;
		ancien_compte_etat = -1;
		ancien_tiers_etat = -1;

		montant_categ_etat = null_real;
		montant_sous_categ_etat = null_real;
		montant_ib_etat = null_real;
		montant_sous_ib_etat = null_real;
		montant_compte_etat = null_real;
		montant_tiers_etat = null_real;
		montant_periode_etat = null_real;
		montant_exo_etat = null_real;
		total_partie = null_real;
		date_debut_periode = NULL;
		exo_en_cours_etat = -1;

		nb_ope_categ_etat = 0;
		nb_ope_sous_categ_etat = 0;
		nb_ope_ib_etat = 0;
		nb_ope_sous_ib_etat = 0;
		nb_ope_compte_etat = 0;
		nb_ope_tiers_etat = 0;
		nb_ope_partie_etat = 0;
		nb_ope_periode_etat = 0;
		nb_ope_exo_etat = 0;

		changement_de_groupe_etat = 0;
		debut_affichage_etat = 1;
		devise_compte_en_cours_etat = 0;

		/* on met ici le pointeur sur les revenus ou sur les dépenses en vérifiant qu'il y en a */
		if (i)
		{
			/* c'est le second passage */
			/* on met le pointeur sur les dépenses */
			if (liste_ope_depenses)
			{
				/* séparation */
				ligne = etat_affiche_affiche_separateur (ligne);
				pointeur_opes = liste_ope_depenses;

				ligne = etat_affiche_affiche_titre_depenses_etat (ligne);
			}
			else
				continue;
		}
		else
		{
			/* c'est le premier passage */
			/* on met le pointeur sur les revenus */
			/* si on n'a pas demandé de séparer les débits et crédits, on ne met pas de titre */
			if (gsb_data_report_get_split_credit_debit (current_report_number))
			{
				/* on sépare les revenus des débits */
				if (liste_ope_revenus)
				{
					pointeur_opes = liste_ope_revenus;

					ligne = etat_affiche_affiche_titre_revenus_etat (ligne);
				}
				else
				{
					/* il n'y a pas de revenus, on saute directement aux dépenses */
					i++;
					pointeur_opes = liste_ope_depenses;

					/* s'il n'y a pas de dépenses non plus, on sort de la boucle */
					if (!liste_ope_depenses)
						continue;

					ligne = etat_affiche_affiche_titre_depenses_etat (ligne);
				}
			}
			else
			{
				/* on ne sépare pas les débits et les crédits, donc pas de titre, juste */
				/* une vérification qu'il y a des opérations */
				if (liste_ope_revenus)
					pointeur_opes = liste_ope_revenus;
				else
					continue;
			}
		}

		/* on commence la boucle qui fait le tour de chaque opé */
		while (pointeur_opes)
		{
			gint transaction_number;
			GsbReal montant;

			transaction_number = gsb_data_transaction_get_transaction_number (pointeur_opes->data);

			pointeur_sort_list = gsb_data_report_get_sorting_type_list (current_report_number);

			while (pointeur_sort_list)
			{
				switch (GPOINTER_TO_INT (pointeur_sort_list->data))
				{
					case 1:
						if (categ_used)
							ligne = etat_affiche_affiche_categ_etat (transaction_number, decalage_categ, ligne);
						break;

					case 2:
						if (categ_used && sous_categ_used)
							ligne = etat_affiche_affiche_sous_categ_etat (transaction_number, decalage_sous_categ, ligne);
						break;

					case 3:
						if (ib_used)
							ligne = etat_affiche_affiche_ib_etat (transaction_number, decalage_ib, ligne);
						break;

					case 4:
						if (ib_used && sous_ib_used)
							ligne = etat_affiche_affiche_sous_ib_etat (transaction_number, decalage_sous_ib, ligne);
						break;

					case 5:
						if (group_reports)
							ligne = etat_affiche_affiche_compte_etat (transaction_number, decalage_compte, ligne);
						break;

					case 6:
						if (payee_used)
							ligne = etat_affiche_affiche_tiers_etat (transaction_number, decalage_tiers, ligne);
				}

				pointeur_sort_list = pointeur_sort_list->next;
			}

			/* on affiche si nécessaire le total de la période */
			ligne = etat_affiche_affiche_total_periode (transaction_number, ligne, 0);

			/* on affiche si nécessaire le total de  l'exercice */
			ligne = etat_affiche_affiche_total_exercice (transaction_number, ligne, 0);

			/* on affiche la ligne de l'opération */
			if (gsb_data_report_get_show_report_transactions (current_report_number))
				ligne = etat_affiche_affichage_ligne_ope (transaction_number, ligne);

			/* on ajoute les montants que pour ceux affichés */
			/* calcule le montant de la categ */
			if (categ_used)
			{
				montant = gsb_data_transaction_get_adjusted_amount_for_currency (transaction_number,
																				 devise_categ_etat,
																				 -1);
				montant_categ_etat = gsb_real_add (montant_categ_etat, montant);
				montant_sous_categ_etat = gsb_real_add (montant_sous_categ_etat, montant);
				nb_ope_categ_etat++;
				nb_ope_sous_categ_etat++;
			}

			/* calcule le montant de l'ib */
			if (ib_used)
			{
				montant = gsb_data_transaction_get_adjusted_amount_for_currency (transaction_number,
																				 devise_ib_etat,
																				 -1);
				montant_ib_etat = gsb_real_add (montant_ib_etat, montant);
				montant_sous_ib_etat = gsb_real_add (montant_sous_ib_etat, montant);
				nb_ope_ib_etat++;
				nb_ope_sous_ib_etat++;
			}

			/* calcule le montant du tiers */
			if (payee_used)
			{
				montant = gsb_data_transaction_get_adjusted_amount_for_currency (transaction_number,
																				 devise_tiers_etat,
																				 -1);
				montant_tiers_etat = gsb_real_add (montant_tiers_etat, montant);
				nb_ope_tiers_etat++;
			}

			/* calcule le montant du compte */
			if (gsb_data_report_get_account_show_amount (current_report_number))
			{
				/* on modifie le montant s'il n'est pas de la devise du compte en cours */
				if (gsb_data_account_get_currency (gsb_data_transaction_get_account_number (transaction_number))
					!= devise_compte_en_cours_etat)
					devise_compte_en_cours_etat = gsb_data_account_get_currency (gsb_data_transaction_get_account_number
																				 (transaction_number));

				montant = gsb_data_transaction_get_adjusted_amount (transaction_number, -1);
				montant_compte_etat = gsb_real_add (montant_compte_etat, montant);
				nb_ope_compte_etat++;
			}

			/* calcule les montants totaux */
			montant = gsb_data_transaction_get_adjusted_amount_for_currency (transaction_number,
																			 devise_generale_etat,
																			 -1);
			total_general = gsb_real_add (total_general, montant);
			nb_ope_general_etat++;

			/* calcule le montant de la periode */
			if (gsb_data_report_get_period_split (current_report_number))
			{
				montant = gsb_data_transaction_get_adjusted_amount_for_currency (transaction_number,
																				 devise_categ_etat,
																				 -1);
				montant_periode_etat = gsb_real_add (montant_periode_etat, montant);
				nb_ope_periode_etat++;
			}
			/* calcule le montant de l'exo */
			if (gsb_data_report_get_financial_year_split (current_report_number))
			{
				montant = gsb_data_transaction_get_adjusted_amount_for_currency (transaction_number,
																				 devise_categ_etat,
																				 -1);
				montant_exo_etat = gsb_real_add (montant_exo_etat, montant);
				nb_ope_exo_etat++;
			}

			total_partie = gsb_real_add (total_partie, montant);
			nb_ope_partie_etat++;
			changement_de_groupe_etat = 0;

			pointeur_opes = pointeur_opes->next;
		}

		/* à la fin, on affiche les totaux des dernières lignes */
		/* on affiche le total de la période en le forçant */
		ligne = etat_affiche_affiche_total_periode (0, ligne, 1);

		/* on affiche le total de l'exercice en le forçant */
		ligne = etat_affiche_affiche_total_exercice (0, ligne, 1);

		ligne = etat_affiche_affiche_totaux_sous_jaccent (GPOINTER_TO_INT (gsb_data_report_get_sorting_type_list
																		   (current_report_number)-> data),
														  ligne);

		/* on ajoute le total de la structure racine */
		switch (GPOINTER_TO_INT (gsb_data_report_get_sorting_type_list (current_report_number)-> data))
		{
			case 1:
				if (categ_used)
					ligne = etat_affiche_affiche_total_categories (ligne);
				break;

			case 2:
				if (categ_used && sous_categ_used)
					ligne = etat_affiche_affiche_total_sous_categ (ligne);
				break;

			case 3:
				if (ib_used)
					ligne = etat_affiche_affiche_total_ib (ligne);
				break;

			case 4:
				if (ib_used && sous_ib_used)
					ligne = etat_affiche_affiche_total_sous_ib (ligne);
				break;

			case 5:
				if (group_reports)
					ligne = etat_affiche_affiche_total_compte (ligne);
				break;

			case 6:
				if (payee_used)
					ligne = etat_affiche_affiche_total_tiers (ligne);
				break;
		}

		/* on affiche le total de la partie en cours */
		/* si les revenus et dépenses ne sont pas mélangés */
		if (gsb_data_report_get_split_credit_debit (current_report_number))
			ligne = etat_affiche_affiche_total_partiel (total_partie, ligne, i);
	}

	/* on affiche maintenant le total général */
	ligne = etat_affiche_affiche_total_general (total_general, ligne);
	nb_lignes = ligne;

	etat_affiche_finish ();
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * Affichage d'un état. Test du nombre d'opérations sélectionnées si trop grand
 * en fonction des choix retour proposé vers préférences
 *
 * \param
 * \param
 * \param
 *
 * \return TRUE if OK FALSE si sortie préférences par annulation
 **/
gboolean affichage_etat (gint report_number,
					 struct EtatAffichage *affichage,
					 gchar *filename)
{
	GSList *liste_opes_selectionnees;
	guint nbre_max_opes = 3000;
	guint nbre_opes;
	gint show_report_transactions;


	devel_debug (NULL);
	if (!report_number)
	{
		report_number = gsb_gui_navigation_get_current_report ();
		if (!report_number)
			return FALSE;
	}

	grisbi_win_status_bar_wait (FALSE);

	if (!affichage)
		affichage = &gtktable_affichage;

	/*   selection des opérations */
	/* on va mettre l'adresse des opés sélectionnées dans une liste */
	liste_opes_selectionnees = recupere_opes_etat (report_number);
	nbre_opes = g_slist_length (liste_opes_selectionnees);

	/* on regarde si il n'y a pas trop d'opérations */
	show_report_transactions = gsb_data_report_get_show_report_transactions (report_number);
	if (show_report_transactions > 0)
		nbre_max_opes = 3000;
#ifndef G_OS_WIN32
	else
		nbre_max_opes = 10000;
#endif /* G_OS_WIN32 */
	if (nbre_opes > nbre_max_opes)
	{
		gint result;

		result = etats_dialog_warning_report_too_big (report_number, nbre_opes, nbre_max_opes);
		if (result != GTK_RESPONSE_OK)		/*on continue */
		{
			result = etats_config_personnalisation_etat ();
			if (result == GTK_RESPONSE_CANCEL)
			{
				gsb_gui_navigation_select_reports_page ();
				g_slist_free (liste_opes_selectionnees);
				grisbi_win_status_bar_stop_wait (FALSE);
				return FALSE;
			}
			else
				return FALSE;
		}
	}

	/* à ce niveau, on a récupéré toutes les opés qui entreront dans */
	/* l'état ; reste plus qu'à les classer et les afficher */
	/* on classe la liste et l'affiche en fonction du choix du type de classement */
	etat_affichage_output = affichage;
	etape_finale_affichage_etat (liste_opes_selectionnees, affichage, filename);
	grisbi_win_status_bar_stop_wait (FALSE);

	return TRUE;
}

/**
 * Affiche un rapport vide pour accélerer l'accès aux préférences du nouvel état'
 *
 * \param report_number		numéro du rapport
 *
 * \return
 **/
void affichage_empty_report (gint report_number)
{
	struct EtatAffichage *affichage;

	if (!report_number)
		return;

	grisbi_win_status_bar_wait (TRUE);
	affichage = &gtktable_affichage;

	/* on classe la liste et l'affiche en fonction du choix du type de classement */
	etat_affichage_output = affichage;
	etape_finale_affichage_etat (NULL, affichage, NULL);
	grisbi_win_status_bar_stop_wait (FALSE);
}

/**
 * cette fontion prend en argument l'adresse d'un état, fait le tour de tous les comptes et
 * sélectionne les opérations qui appartiennent à cet état. elle renvoie une liste des
 * adresses de ces opérations
 * elle est appelée pour l'affichage d'un état ou pour la récupération des tiers d'un état
 *
 * \param report_number		numéro du rapport
 *
 * \return
 **/
GSList *recupere_opes_etat (gint report_number)
{
	GSList *transactions_report_list;
	gint no_exercice_recherche;
	GSList *tmp_list;
	gint ignore_archives;

	transactions_report_list = NULL;

	/* on récupère ignore_archives qui s'il vaut 1 ne retient que la liste courte des opérations */
	ignore_archives = gsb_data_report_get_ignore_archives (report_number);

	/* si on utilise l'exercice courant ou précédent, on cherche ici le numéro de l'exercice correspondant */
	no_exercice_recherche = 0;

	if (gsb_data_report_get_use_financial_year (report_number))
	{
		gint fyear_number;
		gint last_fyear_number;

		/* get the current financial year */
		fyear_number = gsb_data_fyear_get_from_date (gdate_today ());

		switch (gsb_data_report_get_financial_year_type (report_number))
		{
			case 1:
				/* want the current financial year */
				if (fyear_number)
					no_exercice_recherche = fyear_number;
				break;
			case 2:
				/* want the last financial year */
				last_fyear_number = gsb_data_fyear_get_previous_financial_year (fyear_number);
				if (last_fyear_number)
					no_exercice_recherche = last_fyear_number;
				break;
		}
	}

	/* si on a utilisé "le plus grand" dans la recherche de texte, c'est ici qu'on recherche */
	/* les plus grands no de chq, de rappr et de pc dans les comptes choisis */
	/* commence par rechercher si on a utilisé "le plus grand" */
	tmp_list = gsb_data_report_get_text_comparison_list (report_number);
	while (tmp_list)
	{
		gint text_comparison_number;

		text_comparison_number = GPOINTER_TO_INT (tmp_list->data);
		if (gsb_data_report_text_comparison_get_first_comparison (text_comparison_number) == 6
			|| gsb_data_report_text_comparison_get_second_comparison (text_comparison_number) == 6)
		{
			/* on utilise "le plus grand" qque part, donc on va remplir les 3 variables */
			GSList *accounts_list;

			dernier_chq = 0;
			dernier_pc = 0;
			dernier_no_rappr = 0;

			accounts_list = gsb_data_account_get_list_accounts ();
			while (accounts_list)
			{
				gint i;

				i = gsb_data_account_get_no_account (accounts_list->data);

				/* on commence par vérifier que le compte fait partie de l'état */
				if (!gsb_data_report_get_account_use_chosen (report_number)
					 || g_slist_index (gsb_data_report_get_account_numbers_list (report_number),
									   GINT_TO_POINTER (i)) != -1)
				{
					GSList *list_tmp_transactions;

					/* on fait le tour de la liste des opés en recherchant le plus grand dans les 3 variables */
					if (ignore_archives)
						list_tmp_transactions = gsb_data_transaction_get_transactions_list ();
					else
						list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

					while (list_tmp_transactions)
					{
						gint transaction_number_tmp;

						transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions->data);
						if (gsb_data_transaction_get_account_number (transaction_number_tmp) == i)
						{
							const gchar *tmp_str;
							gint tmp_number;

							/* commence par le cheque, il faut que le type opé soit à incrémentation auto */
							/* et le no le plus grand */
							/* on ne recherche le type d'opé que si l'opé a un contenu du type et que celui ci */
							/* est > que dernier_chq */
							if ((tmp_str = gsb_data_transaction_get_method_of_payment_content (transaction_number_tmp)))
							{
								gint payment_number;

								payment_number = gsb_data_transaction_get_method_of_payment_number (transaction_number_tmp);
								if (gsb_data_payment_get_show_entry (payment_number)
									&& gsb_data_payment_get_automatic_numbering (payment_number))
								{
									tmp_number = utils_str_atoi (tmp_str);
									if (tmp_number > dernier_chq)
										dernier_chq = tmp_number;
								}
							}

							/* on récupère maintenant la plus grande pc */
							if ((tmp_str = gsb_data_transaction_get_voucher (transaction_number_tmp)))
							{
								tmp_number = utils_str_atoi (tmp_str);
								if (tmp_number > dernier_pc)
									dernier_pc = tmp_number;
							}

							/* on récupère maintenant le dernier relevé */
							tmp_number = gsb_data_transaction_get_reconcile_number (transaction_number_tmp);
							if (tmp_number > dernier_no_rappr)
								dernier_no_rappr = tmp_number;
						}
						list_tmp_transactions = list_tmp_transactions->next;
					}
				}
				accounts_list = accounts_list->next;
			}
			tmp_list = NULL;
		}
		else
			tmp_list = tmp_list->next;
	}

	/* on traite les comptes concernés */
	tmp_list = gsb_data_account_get_list_accounts ();
	while (tmp_list)
	{
		gint i;

		i = gsb_data_account_get_no_account (tmp_list->data);
		if (!gsb_data_report_get_account_use_chosen (report_number)
			 || g_slist_index (gsb_data_report_get_account_numbers_list (report_number),
							   GINT_TO_POINTER (i)) != -1)
		{
			/* le compte est bon, passe à la suite de la sélection */
			/* on va faire le tour de toutes les opés du compte */
			GSList *list_tmp_transactions;

			if (ignore_archives)
				list_tmp_transactions = gsb_data_transaction_get_transactions_list ();
			else
				list_tmp_transactions = gsb_data_transaction_get_complete_transactions_list ();

			while (list_tmp_transactions)
			{
				gint transaction_number_tmp;

				transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions->data);
				if (gsb_data_transaction_get_account_number (transaction_number_tmp) == i)
				{
					/* si c'est une opé ventilée, dépend de la conf */
					if (gsb_data_transaction_get_split_of_transaction (transaction_number_tmp))
					{
						if (gsb_data_report_get_category_detail_used (report_number))
							goto operation_refusee;

						if (!gsb_data_report_get_not_detail_split (report_number))
							goto operation_refusee;

					}

					if (gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp))
					{
						if (!gsb_data_report_get_category_detail_used (report_number))
						{
							if (gsb_data_report_get_not_detail_split (report_number))
								goto operation_refusee;
						}

					}

					/* on vérifie ensuite si un texte est recherché */
					if (gsb_data_report_get_text_comparison_used (report_number))
					{
						gint garde_ope;
						GSList *comparison_list;

						comparison_list = gsb_data_report_get_text_comparison_list (report_number);
						garde_ope = 0;
						while (comparison_list)
						{
							const gchar *texte;
							gint ope_dans_test;
							gint text_comparison_number;

							text_comparison_number = GPOINTER_TO_INT (comparison_list->data);

							/* on commence par récupérer le texte du champs recherché */
							texte = recupere_texte_test_etat (transaction_number_tmp,
															  gsb_data_report_text_comparison_get_field
															  (text_comparison_number));

							/* à ce niveau, texte est soit null, soit contient le texte dans lequel on effectue la recherche */
							/* on vérifie maintenant en fontion de l'opérateur */
							/* si c'est un chq ou une pc et que use_txt = TRUE, on utilise leur no */
							if ((gsb_data_report_text_comparison_get_field (text_comparison_number) == 8
								 || gsb_data_report_text_comparison_get_field (text_comparison_number) == 9
								 || gsb_data_report_text_comparison_get_field (text_comparison_number) == 10)
								&& !gsb_data_report_text_comparison_get_use_text (text_comparison_number))
							{
								if (texte)
									ope_dans_test = verifie_chq_test_etat (text_comparison_number, texte);
								else
									ope_dans_test = 0;
							}
							else
								ope_dans_test = verifie_texte_test_etat (text_comparison_number, texte);

							/* à ce niveau, ope_dans_test=1 si l'opé a passé ce test */
							/* il faut qu'on fasse le lien avec la ligne précédente */
							switch (gsb_data_report_text_comparison_get_link_to_last_text_comparison (text_comparison_number))
							{
								case -1:
									/* 1ère ligne  */
									garde_ope = ope_dans_test;
									break;

								case 0:
									/* et  */
									garde_ope = garde_ope && ope_dans_test;
									break;

								case 1:
									/* ou  */
									garde_ope = garde_ope || ope_dans_test;
									break;

								case 2:
									/* sauf  */
									garde_ope = garde_ope && (!ope_dans_test);
									break;
							}
							comparison_list = comparison_list->next;
						}

						/* on ne garde l'opération que si garde_ope = 1 */
						if (!garde_ope)
							goto operation_refusee;
					}

					/* on vérifie les R */
					if (gsb_data_report_get_show_m (report_number))
					{
						if (gsb_data_report_get_show_m (report_number) == 1
							&& gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_RAPPROCHEE)
						{
							goto operation_refusee;
						}
						if (gsb_data_report_get_show_m (report_number) == 2)
						{
							if (!gsb_data_transaction_get_marked_transaction (transaction_number_tmp))
								goto operation_refusee;

							if (gsb_data_report_get_show_p (report_number) == 0
								&& gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_POINTEE)
								goto operation_refusee;

							if (gsb_data_report_get_show_r (report_number) == 0
								&& gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_RAPPROCHEE)
								goto operation_refusee;

							if (gsb_data_report_get_show_t (report_number) == 0
								&&
								gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_TELEPOINTEE)
								goto operation_refusee;
						}
					}

					/* vérification du montant nul */
					if (gsb_data_report_get_amount_comparison_only_report_non_null (report_number)
						&& !gsb_data_transaction_get_amount (transaction_number_tmp).mantissa)
						goto operation_refusee;

					/* vérification des montants */
					if (gsb_data_report_get_amount_comparison_used (report_number))
					{
						gint garde_ope;
						GSList *comparison_list;

						comparison_list = gsb_data_report_get_amount_comparison_list (report_number);
						garde_ope = 0;
						while (comparison_list)
						{
							GsbReal montant;
							gint ope_dans_premier_test;
							gint ope_dans_second_test;
							gint ope_dans_test;
							gint amount_comparison_number;

							amount_comparison_number = GPOINTER_TO_INT (comparison_list->data);
							montant = gsb_data_transaction_get_adjusted_amount (transaction_number_tmp, -1);

							/* on vérifie maintenant en fonction de la ligne de test si on garde cette opé */
							ope_dans_premier_test = compare_montants_etat (montant,
																		   gsb_data_report_amount_comparison_get_first_amount
																		   (amount_comparison_number),
																		   gsb_data_report_amount_comparison_get_first_comparison
																		   (amount_comparison_number));

							if (gsb_data_report_amount_comparison_get_link_first_to_second_part (amount_comparison_number) != 3)
								ope_dans_second_test = compare_montants_etat (montant,
																			  gsb_data_report_amount_comparison_get_second_amount
																			  (amount_comparison_number),
																			  gsb_data_report_amount_comparison_get_second_comparison
																			  (amount_comparison_number));
							else
							/* pour éviter les warning lors de la compil */
								ope_dans_second_test = 0;

							switch (gsb_data_report_amount_comparison_get_link_first_to_second_part (amount_comparison_number))
							{
								case 0:
									/* et  */
									ope_dans_test = ope_dans_premier_test && ope_dans_second_test;
									break;

								case 1:
									/*  ou */
									ope_dans_test = ope_dans_premier_test || ope_dans_second_test;
									break;

								case 2:
									/* sauf  */
									ope_dans_test = ope_dans_premier_test && (!ope_dans_second_test);
									break;

								case 3:
									/* aucun  */
									ope_dans_test = ope_dans_premier_test;
									break;

								default:
									ope_dans_test = 0;
							}

							/* à ce niveau, ope_dans_test=1 si l'opé a passé ce test */
							/* il faut qu'on fasse le lien avec la ligne précédente */
							switch (gsb_data_report_amount_comparison_get_link_to_last_amount_comparison (amount_comparison_number))
							{
								case -1:
									/* 1ère ligne  */
									garde_ope = ope_dans_test;
									break;

								case 0:
									/* et  */
									garde_ope = garde_ope && ope_dans_test;
									break;

								case 1:
									/* ou  */
									garde_ope = garde_ope || ope_dans_test;
									break;

								case 2:
									/* sauf  */
									garde_ope = garde_ope && (!ope_dans_test);
									break;
								}
								comparison_list = comparison_list->next;
						}

						/* on ne garde l'opération que si garde_ope = 1 */
						if (!garde_ope)
							goto operation_refusee;
					}

					/* on vérifie les virements */
					if (gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp) > 0)
					{
						if (!gsb_data_report_get_transfer_choice (report_number))
							goto operation_refusee;

						if (gsb_data_report_get_transfer_choice (report_number)== 1)
						{
							/* on inclue l'opé que si le compte de virement */
							/* est un compte de passif ou d'actif */
							if (gsb_data_account_get_kind (gsb_data_transaction_get_contra_transaction_account
														   (transaction_number_tmp)) != GSB_TYPE_LIABILITIES
								&&
								gsb_data_account_get_kind (gsb_data_transaction_get_contra_transaction_account
														   (transaction_number_tmp)) != GSB_TYPE_ASSET)
							goto operation_refusee;

						}
						else
						{
							if (gsb_data_report_get_transfer_choice (report_number)== 2)
							{
								/* on inclut l'opé que si le compte de virement n'est pas présent dans l'état */
								/* si on ne détaille pas les comptes, on ne cherche pas, l'opé est refusée */
								if (gsb_data_report_get_account_use_chosen (report_number))
								{
									if (g_slist_index (gsb_data_report_get_account_numbers_list (report_number),
													   GINT_TO_POINTER (gsb_data_transaction_get_contra_transaction_account
																		(transaction_number_tmp))) != -1)
									goto operation_refusee;
								}
								else
									goto operation_refusee;
							}
							else
							{
								/* on inclut l'opé que si le compte de virement est dans la liste */
								if (g_slist_index (gsb_data_report_get_transfer_account_numbers_list (report_number),
												   GINT_TO_POINTER (gsb_data_transaction_get_contra_transaction_account
																	(transaction_number_tmp))) == -1)
									goto operation_refusee;
							}
						}
					}
					else
					{
						/* l'opé n'est pas un virement, si on doit exclure les non virement, c'est ici */
						if (gsb_data_report_get_transfer_choice (report_number)
							&& gsb_data_report_get_transfer_reports_only (report_number))
							goto operation_refusee;
					}

					/* check the categ only if it's not a split or transfer */
					if (!gsb_data_transaction_get_split_of_transaction (transaction_number_tmp)
						&& gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp) == 0
						&& gsb_data_report_get_category_detail_used (report_number) /* it's a normal categ (or not categ) */
						&& !gsb_data_report_check_categ_budget_in_report (gsb_data_report_get_category_struct_list (report_number),
																		  gsb_data_transaction_get_category_number
																		  (transaction_number_tmp),
																		  gsb_data_transaction_get_sub_category_number
																		  (transaction_number_tmp)))
						goto operation_refusee;

					/* check the buget */
					if ((gsb_data_report_get_budget_detail_used (report_number)
						 &&
						 !gsb_data_report_check_categ_budget_in_report (gsb_data_report_get_budget_struct_list (report_number),
																		gsb_data_transaction_get_budgetary_number
																		(transaction_number_tmp),
																		gsb_data_transaction_get_sub_budgetary_number
																		(transaction_number_tmp))))
						goto operation_refusee;

					/* vérification du tiers */
					if (gsb_data_report_get_payee_detail_used (report_number)
						&&
						g_slist_index (gsb_data_report_get_payee_numbers_list (report_number),
									   GINT_TO_POINTER (gsb_data_transaction_get_payee_number
														(transaction_number_tmp))) == -1)
						goto operation_refusee;

					/* vérification du type d'opération */
					if (gsb_data_report_get_method_of_payment_used (report_number))
					{
						gint payment_number;

						payment_number = gsb_data_transaction_get_method_of_payment_number (transaction_number_tmp);
						if (!payment_number)
							goto operation_refusee;

						if (!g_slist_find_custom (gsb_data_report_get_method_of_payment_list (report_number),
												  gsb_data_payment_get_name (payment_number),
												  (GCompareFunc) utils_str_search_str_in_string_list))
							goto operation_refusee;
					}

					/* vérifie la plage de date */
					if (gsb_data_report_get_use_financial_year (report_number))
					{
						/* on utilise l'exercice */
						if (((gsb_data_report_get_financial_year_type (report_number) == 1
							  || gsb_data_report_get_financial_year_type (report_number) == 2)
							 && (gsb_data_transaction_get_financial_year_number (transaction_number_tmp)!= no_exercice_recherche
								 ||
								 !gsb_data_transaction_get_financial_year_number (transaction_number_tmp)))
							|| (gsb_data_report_get_financial_year_type (report_number) == 3
								&& (g_slist_index (gsb_data_report_get_financial_year_list (report_number),
												   GINT_TO_POINTER (gsb_data_transaction_get_financial_year_number
																	(transaction_number_tmp))) == -1
									||
									!gsb_data_transaction_get_financial_year_number (transaction_number_tmp))))
							goto operation_refusee;
					}
					else
					{
						/* on utilise une plage de dates */
						GDate *date_jour;
						GDate *date_tmp;
						const GDate *date_transaction;

						date_jour = gdate_today ();

						/* on récupère la date ou la date de valeur */
						if (gsb_data_report_get_date_select_value (report_number))
							date_transaction = gsb_data_transaction_get_value_date_or_date (transaction_number_tmp);
						else
							date_transaction = gsb_data_transaction_get_date (transaction_number_tmp);

						switch (gsb_data_report_get_date_type (report_number))
						{
							case 0:
							/* toutes */
							break;

							case 1:
							/* plage perso */
							if (!gsb_data_report_get_personal_date_start (report_number)
								|| !gsb_data_report_get_personal_date_end (report_number)
								|| g_date_compare (gsb_data_report_get_personal_date_start (report_number), date_transaction) > 0
								|| g_date_compare (gsb_data_report_get_personal_date_end (report_number), date_transaction) < 0)
								goto operation_refusee;
							break;

							case 2:
							/* cumul à ce jour */
							if (g_date_compare (date_jour,
												gsb_data_transaction_get_date (transaction_number_tmp)) < 0)
								goto operation_refusee;
							break;

							case 3:
							/* mois en cours */
							if (g_date_get_month (date_jour) != g_date_get_month (date_transaction)
								|| g_date_get_year (date_jour) != g_date_get_year (date_transaction))
								goto operation_refusee;
							break;

							case 4:
							/* année en cours */
							if (g_date_get_year (date_jour) != g_date_get_year (date_transaction))
								goto operation_refusee;
							break;

							case 5:
							/* cumul mensuel */
							if (g_date_get_month (date_jour) != g_date_get_month (date_transaction)
								|| g_date_get_year (date_jour) != g_date_get_year (date_transaction)
								|| g_date_compare (date_jour, date_transaction) < 0)
								goto operation_refusee;
							break;

							case 6:
							/* cumul annuel */
							if (g_date_get_year (date_jour) != g_date_get_year (date_transaction)
								|| g_date_compare (date_jour, date_transaction) < 0)
								goto operation_refusee;
							break;

							case 7:
							/* mois précédent */
							g_date_subtract_months (date_jour, 1);
							if (g_date_get_month (date_jour) != g_date_get_month (date_transaction)
								|| g_date_get_year (date_jour) != g_date_get_year (date_transaction))
								goto operation_refusee;
							break;

							case 8:
							/* année précédente */
							g_date_subtract_years (date_jour, 1);
							if (g_date_get_year (date_jour) != g_date_get_year (date_transaction))
								goto operation_refusee;
							break;

							case 9:
							/* 30 derniers jours */
							date_tmp = g_date_new_dmy (g_date_get_day (date_jour),
													   g_date_get_month (date_jour),
													   g_date_get_year (date_jour));
							g_date_subtract_days (date_tmp, 30);
							if (g_date_compare (date_tmp, date_transaction) > 0
								|| g_date_compare (date_jour, date_transaction) < 0)
								goto operation_refusee;
							break;

							case 10:
							/* 3 derniers mois */
							date_tmp = g_date_new_dmy (g_date_get_day (date_jour),
													   g_date_get_month (date_jour),
													   g_date_get_year (date_jour));
							g_date_subtract_months (date_tmp, 3);
							if (g_date_compare (date_tmp, date_transaction) > 0
								|| g_date_compare (date_jour, date_transaction) < 0)
								goto operation_refusee;
							break;

							case 11:
							/* 6 derniers mois */
							date_tmp = g_date_new_dmy (g_date_get_day (date_jour),
													   g_date_get_month (date_jour),
													   g_date_get_year (date_jour));
							g_date_subtract_months (date_tmp, 6);
							if (g_date_compare (date_tmp, date_transaction) > 0
								||
								g_date_compare (date_jour, date_transaction) < 0)
								goto operation_refusee;
							break;

							case 12:
							/* 12 derniers mois */
							date_tmp = g_date_new_dmy (g_date_get_day (date_jour),
													   g_date_get_month (date_jour),
													   g_date_get_year (date_jour));
							g_date_subtract_months (date_tmp, 12);
							if (g_date_compare (date_tmp, date_transaction) > 0
								|| g_date_compare (date_jour, date_transaction) < 0)
								goto operation_refusee;
							break;
						}
					}
					transactions_report_list = g_slist_append (transactions_report_list,
															   gsb_data_transaction_get_pointer_of_transaction
															   (transaction_number_tmp));
				}
operation_refusee:
				list_tmp_transactions = list_tmp_transactions->next;
			}
		}
		tmp_list = tmp_list->next;
	}

	return (transactions_report_list);
}

/**
 * Fonction de rafraichissement de l'état
 *
 * \param report_number		numéro du rapport
 *
 * \return TRUE if OK FALSE si affichage_etat sort après annulation préférences
 **/
gboolean rafraichissement_etat (gint report_number)
{
	gboolean result;
	devel_debug_int (report_number);

	if (!report_number)
		report_number = gsb_gui_navigation_get_current_report ();

	result = affichage_etat (report_number, &gtktable_affichage, NULL);

	return result;
}

/**
 * met tous les anciens_x_etat sous jaccents à l'origine à -1
 *
 * \param
 *
 * \return
 **/
void denote_struct_sous_jaccentes (gint origine)
{
	GSList *pointeur_glist;
	gint current_report_number;

	current_report_number = gsb_gui_navigation_get_current_report ();

	/* on peut partir du bout de la liste pour revenir vers la structure demandée */
	/* gros vulgaire copier coller de la fonction précédente */
	pointeur_glist = g_slist_reverse (g_slist_copy (gsb_data_report_get_sorting_type_list (current_report_number)));

	while (GPOINTER_TO_INT (pointeur_glist->data) != origine)
	{
		switch (GPOINTER_TO_INT (pointeur_glist->data))
		{
			case 1:
				ancienne_categ_etat = -1;
				ancienne_categ_speciale_etat = 0;
			break;

			case 2:
				ancienne_sous_categ_etat = -1;
			break;

			case 3:
				ancienne_ib_etat = -1;
			break;

			case 4:
				ancienne_sous_ib_etat = -1;
			break;

			case 5:
				ancien_compte_etat = -1;
			break;

			case 6:
				ancien_tiers_etat = -1;
			break;
		}
		pointeur_glist = pointeur_glist->next;
	}
	g_slist_free (pointeur_glist);
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
