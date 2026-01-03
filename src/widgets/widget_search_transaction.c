/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org                                               */
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_search_transaction.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_account.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_payee.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_transactions_list.h"
#include "menu.h"
#include "navigation.h"
#include "transaction_list.h"
#include "transaction_list_select.h"
#include "structures.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "widget_account_property.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint				search_active = -1;
static gint				search_result = 0; 
static gchar *			old_recherche = NULL;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetSearchTransactionPrivate	WidgetSearchTransactionPrivate;

struct _WidgetSearchTransactionPrivate
{
	GtkWidget *			vbox_widget_search_transaction;
	GtkWidget *			button_next;
	GtkWidget *			button_previous;
    GtkWidget *         button_search;
	GtkWidget *			image_cancel;
	GtkWidget *			image_search;

	GtkWidget *			box_other_account;
	GtkWidget *			box_result;
	GtkWidget *			box_search_amount;
	GtkWidget *			checkbutton_backwards_search;
	GtkWidget *			checkbutton_delta_amount;
	GtkWidget *			checkbutton_ignore_case;
	GtkWidget *			checkbutton_search_archive;
	GtkWidget *			combo_other_account;
	GtkWidget *			entry_search_str;
	GtkWidget *			label_devise;
	GtkWidget *			label_search_info;
	GtkWidget *			radiobutton_search_all;
	GtkWidget *			radiobutton_search_amount;
	GtkWidget *			radiobutton_search_note;
	GtkWidget *			radiobutton_search_payee;
	GtkWidget *			spinbutton_delta_amount;

	gint				account_number;
	gboolean			backwards_search;		/* du plus recent au plus ancien = defaut */
	gboolean			delta_amount;
	gint				display_nb_rows;		/* nbre de lignes initialement affichées */
	gboolean			display_r;
	gboolean			ignore_case;
	gboolean			ignore_sign;
	gboolean			search_archive;
	gint				search_type;			/* 1 = payee, 2 = note, 3 = all, 4 = amount */

	GSList *			list;					/* liste des resultats de la recherche */

	gboolean			file_is_modified;
	glong				file_modified;

};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetSearchTransaction, widget_search_transaction, GTK_TYPE_DIALOG)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * reset la recherche
 *
 * \param
 *
 * \return
 **/
static void widget_search_transaction_reset_search (WidgetSearchTransactionPrivate *priv)
{
	/* on retablit le bouton search */
	gtk_button_set_image (GTK_BUTTON (priv->button_search), priv->image_search);
	gtk_button_set_label (GTK_BUTTON (priv->button_search), _("Find"));

	/* on insensibilise les boutons next et previous */
	gtk_widget_set_sensitive (priv->button_next, FALSE);
	gtk_widget_set_sensitive (priv->button_previous, FALSE);

	/* on insensibilise le combo other_account */
	gtk_widget_set_sensitive (priv->box_other_account, FALSE);

	/* on reinitialise la recherche */
	search_active = -1;
	gtk_widget_hide (priv->box_result);
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
static gint widget_search_transaction_sort_result (gconstpointer a,
												   gconstpointer b,
												   WidgetSearchTransactionPrivate *priv)
{
    const GDate *value_date_1 = NULL;
    const GDate *value_date_2 = NULL;
	gint return_value;
	gint transaction_number_1;
    gint transaction_number_2;

	transaction_number_1 = GPOINTER_TO_INT (a);
	transaction_number_2 = GPOINTER_TO_INT (b);

	value_date_1 = gsb_data_transaction_get_value_date_or_date (transaction_number_1);
	value_date_2 = gsb_data_transaction_get_value_date_or_date (transaction_number_2);

	return_value = g_date_compare (value_date_2, value_date_1);

	if (!return_value)
		return_value = transaction_number_2 - transaction_number_1;

	if (priv->backwards_search)
		return_value = -return_value;

	return return_value;
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
static void widget_search_transaction_select_archived (gint transaction_number,
													   gint account_number,
													   gint archive_number)
{
	gint mother_transaction;

	/* re-filter the tree view because if we go directly into the report
	 * and the model was never filtered, we have a nice crash */
	transaction_list_filter (account_number);

	/* if it's an archived transaction, open the archive */
	gsb_transactions_list_add_transactions_from_archive (archive_number, account_number, FALSE);

	/* récupération de la ligne de l'opé dans la liste ; affichage de toutes les opé si nécessaire */
	if (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
		 && !gsb_data_account_get_r (account_number))
	{
		gsb_data_account_set_r (account_number, TRUE);
		gsb_menu_update_view_menu (account_number);
		gsb_transactions_list_mise_a_jour_affichage_r (TRUE);
	}

	/* if it's a child, open the mother */
	mother_transaction = gsb_data_transaction_get_mother_transaction_number (transaction_number);
	if (mother_transaction)
		gsb_transactions_list_switch_expander (mother_transaction);

	transaction_list_select (transaction_number);
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
static void widget_search_transaction_warning_msg (GtkWindow *parent,
												   const gchar *text,
												   const gchar *hint)
{
    GtkWidget *dialog;
	gchar *tmp_markup_str;

    tmp_markup_str = g_markup_printf_escaped ("<span size=\"larger\" weight=\"bold\">%s</span>\n\n", hint);

    dialog = gtk_message_dialog_new (GTK_WINDOW (parent),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_WARNING,
									 GTK_BUTTONS_CLOSE,
									 NULL);

	gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), tmp_markup_str);
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", text);

    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_transient_for  (GTK_WINDOW (dialog), GTK_WINDOW (parent));
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean widget_search_transaction_amount_is_valide (gint transaction_number,
															GsbReal number,
															WidgetSearchTransactionPrivate *priv)
{
	GsbReal amount;

	amount = utils_real_get_calculate_entry (priv->entry_search_str);
	if (priv->delta_amount)
	{
		GsbReal number_sup;
		GsbReal number_inf;
		GsbReal spin_number;

		spin_number = gsb_real_double_to_real (gtk_spin_button_get_value (GTK_SPIN_BUTTON
																		  (priv->spinbutton_delta_amount)));
		number_inf = gsb_real_sub (amount, spin_number);
		number_sup = gsb_real_add (amount, spin_number);

		if (gsb_real_cmp (number, amount) == 0)
			return TRUE;

		if (gsb_real_cmp (number, number_inf) == 1 && gsb_real_cmp (number, number_sup) == -1)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		if (gsb_real_cmp (number, amount) == 0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
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
static gboolean widget_search_transaction_str_is_valide (gint transaction_number,
														 const gchar *text,
														 WidgetSearchTransactionPrivate *priv)
{
	const gchar *tmp_str;
	gint mother_number;
	gint new_search_type;

	/* on ne s'occupera pas des opérations filles si on cherche dans tiers */
	mother_number = gsb_data_transaction_get_mother_transaction_number (transaction_number);

	if (priv->ignore_case)
		new_search_type = priv->search_type +4;
	else
		new_search_type = priv->search_type;

	switch (new_search_type)
	{
		case 1:		/* ignore case = FALSE search_type = payee pas d'operation fille */
			if (mother_number)
			{
				return FALSE;
			}

			tmp_str = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
											  (transaction_number),
											  TRUE);
			if (tmp_str && g_strstr_len (tmp_str, -1, text))
			{
				return TRUE;
			}
			break;

		case 2:		/* ignore case = FALSE search_type = note */
			tmp_str = gsb_data_transaction_get_notes (transaction_number);

			if (tmp_str && g_strstr_len (tmp_str, -1, text))
			{
				return TRUE;
			}
			break;

		case 3:		/* ignore case = FALSE search_type = all */
			if (!mother_number)
			{
				tmp_str = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
												  (transaction_number),
												  TRUE);
				if (tmp_str && g_strstr_len (tmp_str, -1, text))
				{
					return TRUE;
				}
			}

			tmp_str = gsb_data_transaction_get_notes (transaction_number);
			if (tmp_str && g_strstr_len (tmp_str, -1, text))
			{
				return TRUE;
			}
			break;

		case 4:		/* inutilise correspond à la recherche d'un montant */
			break;
		
		case 5:		/* ignore case = TRUE, search_type = payee, pas d'operation fille */
			if (mother_number)
			{
				return FALSE;
			}

			tmp_str = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
											  (transaction_number),
											  TRUE);
			if (tmp_str && utils_str_my_case_strstr (tmp_str, text))
			{
				return TRUE;
			}

			break;

		case 6:		/* ignore case = TRUE, search_type = note */
			tmp_str = gsb_data_transaction_get_notes (transaction_number);
			if (tmp_str && utils_str_my_case_strstr (tmp_str, text))
			{
				return TRUE;
			}
			break;

		default:	/* ignore case = TRUE search_type = all */
			if (!mother_number)
			{
				tmp_str = gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
												  (transaction_number),
												  TRUE);
				if (tmp_str && utils_str_my_case_strstr (tmp_str, text))
				{
					return TRUE;
				}
			}

			tmp_str = gsb_data_transaction_get_notes (transaction_number);
			if (tmp_str && utils_str_my_case_strstr (tmp_str, text))
			{
				return TRUE;
			}
	};

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
static GSList *widget_search_transaction_get_list (gint account_number,
												   const gchar *text,
												   WidgetSearchTransaction *dialog)
{
	GSList *list = NULL;
	GSList *tmp_list;
	WidgetSearchTransactionPrivate *priv;

	priv = widget_search_transaction_get_instance_private (dialog);
	if (priv->search_archive)
		tmp_list = gsb_data_transaction_get_complete_transactions_list ();
	else
		tmp_list = gsb_data_transaction_get_transactions_list ();
	while (tmp_list)
	{
		gint transaction_number;
		TransactionStruct *transaction;

		transaction = tmp_list->data;

		transaction_number = gsb_data_transaction_get_transaction_number (transaction);
		if (gsb_data_transaction_get_account_number (transaction_number) == account_number)
		{
			if (priv->search_type == 4)
			{
				GsbReal number;

				number = gsb_data_transaction_get_adjusted_amount (transaction_number, -1);
				if (widget_search_transaction_amount_is_valide (transaction_number, number, priv))
				{
					list = g_slist_append (list, GINT_TO_POINTER (transaction_number));
				}
				if (priv->ignore_sign)
				{
					if (widget_search_transaction_amount_is_valide (transaction_number, gsb_real_opposite (number), priv))
					{
						list = g_slist_append (list, GINT_TO_POINTER (transaction_number));
					}
				}					
			}
			else if (widget_search_transaction_str_is_valide (transaction_number, text, priv))
			{
				list = g_slist_append (list, GINT_TO_POINTER (transaction_number));
			}
		}
		tmp_list = tmp_list->next;
	}

	/* tri de la liste en fonction des dates */
	list = g_slist_sort_with_data (list, (GCompareDataFunc) widget_search_transaction_sort_result, priv);

	return list;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_transaction_button_next_clicked (GtkButton *button,
														   WidgetSearchTransactionPrivate *priv)
{
	gint transaction_number;
	gint archive_number = 0;

	transaction_number = GPOINTER_TO_INT (g_slist_nth_data (priv->list, search_active-1));
	if (priv->search_archive)
		archive_number = gsb_data_transaction_get_archive_number (transaction_number);
	if (archive_number)
		widget_search_transaction_select_archived (transaction_number, priv->account_number, archive_number);
	else
		transaction_list_select (transaction_number);
	search_active--;

	if (search_active == search_result - 1)
		gtk_widget_set_sensitive (priv->button_previous, FALSE);
	else
		gtk_widget_set_sensitive (priv->button_previous, TRUE);

	if (search_active == 0)
		gtk_widget_set_sensitive (priv->button_next, FALSE);
	else
		gtk_widget_set_sensitive (priv->button_next, TRUE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_transaction_button_previous_clicked (GtkButton *button,
															   WidgetSearchTransactionPrivate *priv)
{
	gint transaction_number;
	gint archive_number = 0;

	transaction_number = GPOINTER_TO_INT (g_slist_nth_data (priv->list, search_active+1));
	if (priv->search_archive)
		archive_number = gsb_data_transaction_get_archive_number (transaction_number);
	if (archive_number)
		widget_search_transaction_select_archived (transaction_number, priv->account_number, archive_number);
	else
		transaction_list_select (transaction_number);
	search_active++;

	if (search_active == search_result - 1)
		gtk_widget_set_sensitive (priv->button_previous, FALSE);
	else
		gtk_widget_set_sensitive (priv->button_previous, TRUE);

	if (search_active == 0)
		gtk_widget_set_sensitive (priv->button_next, FALSE);
	else
		gtk_widget_set_sensitive (priv->button_next, TRUE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_transaction_button_search_clicked (GtkButton *button,
															 WidgetSearchTransaction *dialog)
{
	const gchar *text;
	gchar *tmp_str;
	gint transaction_number;
	gint archive_number = 0;
	WidgetSearchTransactionPrivate *priv;

	priv = widget_search_transaction_get_instance_private (dialog);

	/* regarde si on a un text avant toute chose */
	text = gtk_entry_get_text (GTK_ENTRY (priv->entry_search_str));
	if (strlen (text) == 0)
	{
		gchar *tmp_str1;
		gchar *tmp_str2;

		widget_search_transaction_reset_search (priv);

		tmp_str1 = g_strdup (_("You must enter a search text"));
		tmp_str2 = g_strdup (_("WARNING"));

		widget_search_transaction_warning_msg (GTK_WINDOW (dialog), tmp_str1, tmp_str2);

		g_free (tmp_str1);
		g_free (tmp_str2);

		return;
	}
	else
	{
		/* on le memorise pour plus tard */
		if (old_recherche)
			g_free (old_recherche);
		old_recherche = g_strdup (text);
	}

	if (search_active == -1)
	{
		/* on remplit list */
		priv->list = widget_search_transaction_get_list (priv->account_number, text, WIDGET_SEARCH_TRANSACTION (dialog));
		search_active = g_slist_length (priv->list);
		search_result = search_active;
		if (search_active > 0)
		{
			gchar *str_number;

			/* on change eventuellement l'affichage pour une ligne */
			gsb_transactions_list_change_aspect_liste (1);
			if (!priv->file_is_modified)
			{
				GrisbiWinRun *w_run;

				w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
				priv->file_modified = w_run->file_modification;
			}

			/* on affiche le résultat */
			str_number = utils_str_itoa (search_active);
			if (search_active == 1)
				tmp_str = g_strdup (_("1 transaction were found"));
			else
				tmp_str = g_strconcat (str_number, _(" transactions were found"), NULL);
			g_free (str_number);

			transaction_number = GPOINTER_TO_INT (g_slist_nth_data (priv->list, search_active-1));
			if (priv->search_archive)
				archive_number = gsb_data_transaction_get_archive_number (transaction_number);
    		if (archive_number)
			{
				widget_search_transaction_select_archived (transaction_number, priv->account_number, archive_number);
				gsb_transactions_list_set_visible_archived_button (TRUE);
			}
			else
				transaction_list_select (transaction_number);

			/* on initialise les boutons next et previous et modifie le bouton search si search_active > 1 */
			if (search_active > 1)
			{
				gtk_button_set_image (GTK_BUTTON (priv->button_search), priv->image_cancel);
				gtk_button_set_label (GTK_BUTTON (priv->button_search), "Stop");
				gtk_widget_set_sensitive (priv->button_next, TRUE);
				gtk_widget_set_sensitive (priv->button_previous, FALSE);
			}

			/* on desensibilise le combo other_account */
			gtk_widget_set_sensitive (priv->box_other_account, FALSE);

			/* on prépare la suite */
			search_active--;
		}
		else
		{
			tmp_str = g_strdup (_("No transaction was found"));
			gtk_widget_set_sensitive (priv->box_other_account, TRUE);
			search_active = -1;
		}

		/* set info label text */
		gtk_label_set_text (GTK_LABEL (priv->label_search_info), tmp_str);
		g_free (tmp_str);

		/* on affiche le resultat */
		gtk_widget_show (priv->box_result);
	}
	else /* on reinitialise la recherche et on cache le resultat */
	{
		widget_search_transaction_reset_search (priv);
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
static void widget_search_transaction_checkbutton_backwards_search_toggled (GtkToggleButton *togglebutton,
																			WidgetSearchTransactionPrivate *priv)
{
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	priv->backwards_search = gtk_toggle_button_get_active (togglebutton);
	w_run->backwards_search = priv->backwards_search;

	/* on reinitialise la recherche */
	widget_search_transaction_reset_search (priv);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_transaction_checkbutton_delta_amount_toggled (GtkToggleButton *togglebutton,
																		WidgetSearchTransactionPrivate *priv)
{
	priv->delta_amount = gtk_toggle_button_get_active (togglebutton);
	gtk_widget_set_sensitive (priv->spinbutton_delta_amount, priv->delta_amount);

	/* on reinitialise la recherche */
	search_active = -1;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_transaction_checkbutton_ignore_case_sign_toggled (GtkToggleButton *togglebutton,
																			WidgetSearchTransactionPrivate *priv)
{
	if (priv->search_type == 4)
		priv->ignore_sign = gtk_toggle_button_get_active (togglebutton);
	else
		priv->ignore_case = gtk_toggle_button_get_active (togglebutton);

	/* on reinitialise la recherche */
	widget_search_transaction_reset_search (priv);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_transaction_checkbutton_search_archive_toggled (GtkToggleButton *togglebutton,
																		  WidgetSearchTransactionPrivate *priv)
{
	priv->search_archive = gtk_toggle_button_get_active (togglebutton);

	/* on reinitialise la recherche */
	widget_search_transaction_reset_search (priv);
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
static gboolean widget_search_transaction_radiobutton_toggled (GtkWidget *button,
															   WidgetSearchTransactionPrivate *priv)
{
	gint button_number;

	button_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "pointer"));

	/* blocage du signal de callback du bouton priv->checkbutton_ignore_case */
	g_signal_handlers_block_by_func (G_OBJECT (priv->checkbutton_ignore_case),
									 G_CALLBACK (widget_search_transaction_checkbutton_ignore_case_sign_toggled),
									 priv);
	switch (button_number)
	{
		case 1:
			gtk_button_set_label (GTK_BUTTON (priv->checkbutton_ignore_case), _("Ignore case"));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), priv->ignore_case);
			gtk_widget_set_sensitive (priv->box_search_amount, FALSE);
			priv->search_type = 1;
			break;
		case 2:
			gtk_button_set_label (GTK_BUTTON (priv->checkbutton_ignore_case), _("Ignore case"));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), priv->ignore_case);
			gtk_widget_set_sensitive (priv->box_search_amount, FALSE);
			priv->search_type = 2;
			break;
		case 3:
			gtk_button_set_label (GTK_BUTTON (priv->checkbutton_ignore_case), _("Ignore case"));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), priv->ignore_case);
			gtk_widget_set_sensitive (priv->box_search_amount, FALSE);
			priv->search_type = 3;
			break;
		case 4:
			gtk_button_set_label (GTK_BUTTON (priv->checkbutton_ignore_case), _("Ignore sign"));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), priv->ignore_sign);
			gtk_widget_set_sensitive (priv->box_search_amount, TRUE);
			priv->search_type = 4;
			break;
	}

	/* deblocage du signal de callback du bouton priv->checkbutton_ignore_case */
	g_signal_handlers_unblock_by_func (G_OBJECT (priv->checkbutton_ignore_case),
									   G_CALLBACK (widget_search_transaction_checkbutton_ignore_case_sign_toggled),
									   priv);

	/* on reinitialise la recherche */
	widget_search_transaction_reset_search (priv);

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
static gboolean widget_search_transaction_entry_key_press_event  (GtkWidget *entry,
																  GdkEventKey *ev,
																  WidgetSearchTransaction *dialog)
{
	switch (ev->keyval)
    {
		case GDK_KEY_KP_Enter :
		case GDK_KEY_Return :
			/* on réinitialise la recherche */
			search_active = -1;
			widget_search_transaction_button_search_clicked (NULL, dialog);
			break;
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
static gboolean widget_search_transaction_spinbutton_delta_amount_key_press_event  (GtkWidget *entry,
																					GdkEventKey *ev,
																					WidgetSearchTransaction *dialog)
{
	WidgetSearchTransactionPrivate *priv;

	priv = widget_search_transaction_get_instance_private (dialog);
	switch (ev->keyval)
    {
		case GDK_KEY_KP_Enter :
		case GDK_KEY_Return :
			/* on réinitialise la recherche */
			widget_search_transaction_reset_search (priv);
			widget_search_transaction_button_search_clicked (NULL, dialog);
			break;

		case GDK_KEY_v:         /* touche v */
			if ((ev->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
			{
				gtk_widget_set_sensitive (priv->button_search, TRUE);
			}
			break;
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
static gboolean widget_search_transaction_entry_lose_focus (GtkWidget *entry,
															GdkEvent  *event,
															WidgetSearchTransactionPrivate *priv)
{
	if (gtk_entry_buffer_get_length (gtk_entry_get_buffer (GTK_ENTRY (entry))))
	{
		const gchar *text;

		text = gtk_entry_get_text (GTK_ENTRY (priv->entry_search_str));

		/* on reinitialise eventuellement la recherche */
		if (old_recherche && strcmp (old_recherche, text))
			widget_search_transaction_reset_search (priv);

		gtk_widget_set_sensitive (priv->button_search, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (priv->button_search, FALSE);
	}

	return FALSE;
}

static void widget_search_transaction_combo_other_account_changed (GtkComboBox *combo,
																   WidgetSearchTransaction *dialog)
{
	GtkTreeIter iter;
	devel_debug (NULL);

	if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter))
	{
		GtkTreeModel *store;
		gint account_number;
		WidgetSearchTransactionPrivate *priv;

		store = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
        gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 1, &account_number, -1);

		priv = widget_search_transaction_get_instance_private (dialog);

		if (account_number == 0)
			return;

		/* on retablit les paramètres de lancien compte */
		gsb_transactions_list_change_aspect_liste (priv->display_nb_rows);
		if (!priv->file_is_modified)
		{
			GrisbiWinRun *w_run;

			w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
			priv->file_modified = w_run->file_modification;
		}
		if (gsb_data_account_get_r (priv->account_number) != priv->display_r)
		{
			gsb_data_account_set_r (priv->account_number, FALSE);
			gsb_menu_update_view_menu (priv->account_number);
			gsb_transactions_list_mise_a_jour_affichage_r (FALSE);
		}

		/* on change pour le nouveau compte */
	    gsb_gui_navigation_change_account (account_number);
	    widget_account_property_fill ();
	    gsb_gui_navigation_set_selection (GSB_ACCOUNT_PAGE, account_number, 0);

		/* on réinitialise la recherche */
		priv->account_number = account_number;
		search_active = -1;
		widget_search_transaction_button_search_clicked (NULL, dialog);
	}
}

/**
 * check if the given row is or not a separator,
 * used in interne in gtk
 *
 * \param model
 * \param iter
 * \param combofix
 *
 * \return TRUE if it's a separator, FALSE else
 **/
static gboolean widget_search_transaction_combo_separator_func (GtkTreeModel *model,
																GtkTreeIter *iter,
																GtkComboBox *combo)
{
    gboolean value;

    gtk_tree_model_get (GTK_TREE_MODEL (model), iter, 2, &value, -1);

    if (value)
	    return TRUE;
    return FALSE;
}

/**
 * init a combobox from ui containing the list of others accounts
 *
 * \param priv
 *
 * \return
 * */
static void widget_search_transaction_init_combo_other_account (WidgetSearchTransactionPrivate *priv)
{
    GSList *list_tmp;
    GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;
	gchar *msg;

    store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

	/* set separator before current account : col 2 of model */
	gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (priv->combo_other_account),
										  (GtkTreeViewRowSeparatorFunc) widget_search_transaction_combo_separator_func,
										  priv->combo_other_account,
										  NULL);
	list_tmp = gsb_data_account_get_list_accounts ();

	/* on demande de choisir un nouveau compte */
	msg = g_strdup (_("Choose another account"));

	gtk_list_store_append (GTK_LIST_STORE (store), &iter);
	gtk_list_store_set (store,
						&iter,
						0, msg,
						1, 0,
						2, FALSE,
						-1);
	list_tmp = list_tmp->next;
	while (list_tmp)
	{
		gint account_number;

		account_number = gsb_data_account_get_no_account (list_tmp->data);

		if (account_number == priv->account_number)
		{
			list_tmp = list_tmp->next;
			continue;
		}
		if (account_number >= 0
			&& (gsb_data_account_get_kind (account_number) != GSB_TYPE_LIABILITIES
				|| gsb_data_account_get_bet_credit_card (account_number)))
		{
			gtk_list_store_append (GTK_LIST_STORE (store), &iter);
				gtk_list_store_set (store,
						 &iter,
						 0, gsb_data_account_get_name (account_number),
						 1, account_number,
						 2, FALSE,
						 -1);
		}
		list_tmp = list_tmp->next;
	}

	/* on ajoute à la fin le compte initial */
	gtk_list_store_append (GTK_LIST_STORE (store), &iter);
	gtk_list_store_set (store, &iter, 2, TRUE, -1); /* set separator before current account*/
	gtk_list_store_append (GTK_LIST_STORE (store), &iter);
	gtk_list_store_set (store,
						&iter,
						0, gsb_data_account_get_name (priv->account_number),
						1, priv->account_number,
						2, FALSE,
						-1);

    gtk_combo_box_set_model (GTK_COMBO_BOX (priv->combo_other_account), GTK_TREE_MODEL (store));

    /* by default, this is blank, so set the first */
    gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_other_account), 0);

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (priv->combo_other_account), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (priv->combo_other_account), renderer, "text", 0, NULL);
}

/**
 * Creation de la boite de recherche
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_transaction_setup_dialog (WidgetSearchTransaction *dialog,
													gint transaction_number)
{
	const gchar *currency_code;
	gint currency;
	GrisbiWinEtat *w_etat;
	GrisbiWinRun *w_run;
	WidgetSearchTransactionPrivate *priv;

	devel_debug_int (transaction_number);
	priv = widget_search_transaction_get_instance_private (dialog);
	priv->account_number = gsb_gui_navigation_get_current_account ();

	/* set currency */
	currency = gsb_data_account_get_currency (priv->account_number);
	currency_code = gsb_data_currency_get_nickname_or_code_iso (currency);
	gtk_label_set_text (GTK_LABEL (priv->label_devise), currency_code);

	/* set types of search */
	priv->search_type = 1;

	/* set number for radiobutton */
    g_object_set_data (G_OBJECT (priv->radiobutton_search_payee), "pointer", GINT_TO_POINTER (1));
    g_object_set_data (G_OBJECT (priv->radiobutton_search_note), "pointer", GINT_TO_POINTER (2));
    g_object_set_data (G_OBJECT (priv->radiobutton_search_all), "pointer", GINT_TO_POINTER (3));
    g_object_set_data (G_OBJECT (priv->radiobutton_search_amount), "pointer", GINT_TO_POINTER (4));

	/* set ignore case label */
	gtk_button_set_label (GTK_BUTTON (priv->checkbutton_ignore_case), _("Ignore case"));

	/* init ignore_case comme dans les preferences du formulaire */
	w_etat = grisbi_win_get_w_etat ();
	if (w_etat->combofix_case_sensitive)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), FALSE);
		priv->ignore_case = FALSE;
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), TRUE);
		priv->ignore_case = TRUE;
	}

	/* set search_archive */
	priv->search_archive = FALSE;

	/* set backwards_search */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_backwards_search), w_run->backwards_search);
	priv->backwards_search = w_run->backwards_search;

	/* save the number of rows */
	priv->display_nb_rows = gsb_data_account_get_nb_rows (priv->account_number);
	priv->display_r = gsb_data_account_get_r (priv->account_number);

	/* set account combo */
	widget_search_transaction_init_combo_other_account (priv);

	/* memorise le statut de modification */
	if (w_run->file_modification)
		priv->file_is_modified = TRUE;
	else
		priv->file_modified = w_run->file_modification;

	/* set signals */
	g_signal_connect (G_OBJECT (priv->button_next),
					  "clicked",
					  G_CALLBACK (widget_search_transaction_button_next_clicked),
					  priv);
	g_signal_connect (G_OBJECT (priv->button_previous),
					  "clicked",
					  G_CALLBACK (widget_search_transaction_button_previous_clicked),
					  priv);
	g_signal_connect (G_OBJECT (priv->button_search),
					  "clicked",
					  G_CALLBACK (widget_search_transaction_button_search_clicked),
					  dialog);
	g_signal_connect (G_OBJECT (priv->checkbutton_backwards_search),
					  "clicked",
					  G_CALLBACK (widget_search_transaction_checkbutton_backwards_search_toggled),
					  priv);
	g_signal_connect (G_OBJECT (priv->checkbutton_delta_amount),
					  "toggled",
					  G_CALLBACK (widget_search_transaction_checkbutton_delta_amount_toggled),
					  priv);
	g_signal_connect (G_OBJECT (priv->checkbutton_ignore_case),
					  "toggled",
					  G_CALLBACK (widget_search_transaction_checkbutton_ignore_case_sign_toggled),
					  priv);
	g_signal_connect (G_OBJECT (priv->checkbutton_search_archive),
					  "toggled",
					  G_CALLBACK (widget_search_transaction_checkbutton_search_archive_toggled),
					  priv);
	g_signal_connect (G_OBJECT (priv->combo_other_account),
					  "changed",
					  G_CALLBACK (widget_search_transaction_combo_other_account_changed),
					  dialog);
	g_signal_connect (G_OBJECT (priv->entry_search_str),
					  "focus-out-event",
					  G_CALLBACK (widget_search_transaction_entry_lose_focus),
					  priv);
	g_signal_connect (G_OBJECT (priv->entry_search_str),
					  "key-press-event",
					  G_CALLBACK (widget_search_transaction_entry_key_press_event),
					  dialog);
	g_signal_connect (G_OBJECT (priv->radiobutton_search_all),
					  "toggled",
					  G_CALLBACK (widget_search_transaction_radiobutton_toggled),
					  priv);
	g_signal_connect (G_OBJECT (priv->radiobutton_search_amount),
					  "toggled",
					  G_CALLBACK (widget_search_transaction_radiobutton_toggled),
					  priv);
	g_signal_connect (G_OBJECT (priv->radiobutton_search_note),
					  "toggled",
					  G_CALLBACK (widget_search_transaction_radiobutton_toggled),
					  priv);
	g_signal_connect (G_OBJECT (priv->radiobutton_search_payee),
					  "toggled",
					  G_CALLBACK (widget_search_transaction_radiobutton_toggled),
					  priv);

	g_signal_connect (G_OBJECT (priv->spinbutton_delta_amount),
					  "key-press-event",
					  G_CALLBACK (widget_search_transaction_spinbutton_delta_amount_key_press_event),
					  dialog);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_search_transaction_init (WidgetSearchTransaction *dialog)
{
	gtk_widget_init_template (GTK_WIDGET (dialog));
}

static void widget_search_transaction_dispose (GObject *object)
{
	GrisbiWinRun *w_run;
	WidgetSearchTransactionPrivate *priv;

	priv = widget_search_transaction_get_instance_private (WIDGET_SEARCH_TRANSACTION (object));
	gsb_transactions_list_change_aspect_liste (priv->display_nb_rows);

	/* on supprime éventuellement la demande de sauvegarde */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	if (!priv->file_is_modified && priv->file_modified >= w_run->file_modification)
	{
		gsb_file_set_modified (FALSE);

	}
	if (gsb_data_account_get_r (priv->account_number) != priv->display_r)
	{
		gsb_data_account_set_r (priv->account_number, FALSE);
		gsb_menu_update_view_menu (priv->account_number);
        gsb_transactions_list_mise_a_jour_affichage_r (FALSE);
	}

	G_OBJECT_CLASS (widget_search_transaction_parent_class)->dispose (object);
}

static void widget_search_transaction_class_init (WidgetSearchTransactionClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_search_transaction_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_search_transaction.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, vbox_widget_search_transaction);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, button_previous);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, button_next);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, button_search);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, image_cancel);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, image_search);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, box_other_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, box_result);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, box_search_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, checkbutton_backwards_search);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, checkbutton_delta_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, checkbutton_ignore_case);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, checkbutton_search_archive);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, combo_other_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, entry_search_str);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, label_devise);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, label_search_info);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, radiobutton_search_all);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, radiobutton_search_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, radiobutton_search_note);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, radiobutton_search_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTransaction, spinbutton_delta_amount);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
WidgetSearchTransaction *widget_search_transaction_new (GtkWidget *win,
														gint *transaction_number)
{
	gint height;
	gint width;
	gint root_x;
	gint root_y;
	GrisbiAppConf *a_conf;
	WidgetSearchTransaction *dialog;

	dialog = g_object_new (WIDGET_SEARCH_TRANSACTION_TYPE, NULL);

	/* set position of dialog */
	a_conf = grisbi_app_get_a_conf ();
    if (!a_conf->low_definition_screen)
	{
		gtk_window_set_gravity (GTK_WINDOW (dialog), GDK_GRAVITY_SOUTH_EAST);
		gtk_window_get_size (GTK_WINDOW (win), &width, &height);
		gtk_window_get_position (GTK_WINDOW (win),&root_x, &root_y);
		gtk_window_move (GTK_WINDOW (dialog), width + (root_x/3), height/3);
	}

	widget_search_transaction_setup_dialog (dialog, GPOINTER_TO_INT (transaction_number));

	return dialog;
}

/**
 *
 *
 * \param dialog
 * \param result_id
 *
 * \return
 **/
void widget_search_transaction_dialog_response  (GtkDialog *dialog,
												 gint result_id)
{
	if (result_id != 1)
	{
		gtk_widget_destroy (GTK_WIDGET (dialog));
	}
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

