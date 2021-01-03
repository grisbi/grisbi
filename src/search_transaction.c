/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          http://www.grisbi.org                                                */
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
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "search_transaction.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_account.h"
#include "gsb_account_property.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_payee.h"
#include "gsb_data_transaction.h"
#include "gsb_transactions_list.h"
#include "menu.h"
#include "navigation.h"
#include "transaction_list.h"
#include "transaction_list_select.h"
#include "structures.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gint				search_active = -1;
static gchar *			old_recherche = NULL;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _SearchTransactionPrivate   SearchTransactionPrivate;

struct _SearchTransactionPrivate
{
	GtkWidget *			vbox_search_transaction;
    GtkWidget *         button_search;

	GtkWidget *			box_other_account;
	GtkWidget *			box_search_amount;
	GtkWidget *			box_search_str;
	GtkWidget *			checkbutton_backwards_search;
	GtkWidget *			checkbutton_delta_amount;
	GtkWidget *			checkbutton_ignore_case;
    GtkWidget *			checkbutton_note;
	GtkWidget *			checkbutton_payee;
	GtkWidget *			checkbutton_search_archive;
	GtkWidget *			combo_other_account;
	GtkWidget *			entry_search_str;
	GtkWidget *			label_devise;
	GtkWidget *			label_search_info;
	GtkWidget *			radiobutton_search_amount;
	GtkWidget *			radiobutton_search_str;
	GtkWidget *			spinbutton_delta_amount;

	gint				account_number;
	gboolean			backwards_search;		/* du plus recent au plus ancien = defaut */
	gboolean			delta_amount;
	gint				display_nb_rows;		/* nbre de lignes initialement affichées */
	gboolean			display_r;
	gboolean			ignore_case;
	gboolean			search_archive;
	gint				search_type;			/* 1 = search_str, 2 = search_amount */
	gint				search_str_type;		/* 1 = payee, 2 = note, 3 = all */

	GSList *			list;					/* liste des resultats de la recherche */
};

G_DEFINE_TYPE_WITH_PRIVATE (SearchTransaction, search_transaction, GTK_TYPE_DIALOG)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gint search_transaction_sort_result (gconstpointer a,
											gconstpointer b,
											SearchTransactionPrivate *priv)
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
static void search_transaction_select_archived (gint transaction_number,
												gint account_number,
												gint archive_number)
{
	gint mother_transaction;

	/* if it's an archived transaction, open the archive */
    archive_number = gsb_data_transaction_get_archive_number (transaction_number);

	/* re-filter the tree view because if we go directly into the report
	 * and the model was never filtered, we have a nice crash */
	transaction_list_filter (account_number);
	gsb_transactions_list_restore_archive (archive_number, FALSE);

	/* récupération de la ligne de l'opé dans la liste ; affichage de toutes les opé si nécessaire */
	if (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
		 &&
		 !gsb_data_account_get_r (account_number))
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
static void search_transaction_warning_msg (GtkWindow *parent,
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
static gboolean search_transaction_amount_is_valide (gint transaction_number,
													 const gchar *text,
													 SearchTransaction *dialog)
{
	GsbReal amount;
	GsbReal number;
	SearchTransactionPrivate *priv;

	priv = search_transaction_get_instance_private (dialog);

	/* on ne cherche pas dans les opérations meres DISCUTABLE */
	if (gsb_data_transaction_get_split_of_transaction (transaction_number))
		return FALSE;

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
		number = gsb_data_transaction_get_adjusted_amount (transaction_number, -1);

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
		number = gsb_data_transaction_get_adjusted_amount (transaction_number, -1);
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
static gboolean search_transaction_str_is_valide (gint transaction_number,
												  const gchar *text,
												  SearchTransaction *dialog)
{
	const gchar *tmp_str;
	gint mother_number;
	gint new_search_type;
	SearchTransactionPrivate *priv;

	priv = search_transaction_get_instance_private (dialog);

	/* on ne s'occupera pas des opérations filles si on cherche dans tiers */
	mother_number = gsb_data_transaction_get_mother_transaction_number (transaction_number);

	if (priv->ignore_case)
		new_search_type = priv->search_str_type +3;
	else
		new_search_type = priv->search_str_type;

	switch (new_search_type)
	{
		case 1:		/* ignore case = FALSE search_type = payee pas d'operation fille */
			if (mother_number)
				return FALSE;

			tmp_str = gsb_data_payee_get_name (gsb_data_transaction_get_party_number
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
				return TRUE;

			break;

		case 3:		/* ignore case = FALSE search_type = all */
			if (!mother_number)
			{
				tmp_str = gsb_data_payee_get_name (gsb_data_transaction_get_party_number
												  (transaction_number),
												  TRUE);
				if (tmp_str && g_strstr_len (tmp_str, -1, text))
					return TRUE;
			}

			tmp_str = gsb_data_transaction_get_notes (transaction_number);
			if (tmp_str && g_strstr_len (tmp_str, -1, text))
				return TRUE;

			break;

		case 4:		/* ignore case = TRUE search_type = payee pas d'operation fille*/
			if (mother_number)
				return FALSE;

			tmp_str = gsb_data_payee_get_name (gsb_data_transaction_get_party_number
											  (transaction_number),
											  TRUE);
			if (tmp_str && utils_str_my_case_strstr (tmp_str, text))
				return TRUE;

			break;

		case 5:		/* ignore case = TRUE search_type = note */
			tmp_str = gsb_data_transaction_get_notes (transaction_number);
			if (tmp_str && utils_str_my_case_strstr (tmp_str, text))
				return TRUE;
			break;

		default:		/* ignore case = TRUE search_type = all */
			if (!mother_number)
			{
				tmp_str = gsb_data_payee_get_name (gsb_data_transaction_get_party_number
												  (transaction_number),
												  TRUE);
				if (tmp_str && g_strstr_len (tmp_str, -1, text))
					return TRUE;
			}

			tmp_str = gsb_data_transaction_get_notes (transaction_number);
			if (tmp_str && utils_str_my_case_strstr (tmp_str, text))
				return TRUE;
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
static GSList *search_transaction_get_list (gint account_number,
											const gchar *text,
											SearchTransaction *dialog)
{
	GSList *list = NULL;
	GSList *tmp_list;
	SearchTransactionPrivate *priv;

	priv = search_transaction_get_instance_private (dialog);
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
			if (priv->search_type == 1 && search_transaction_str_is_valide (transaction_number, text, dialog))
			{
				list = g_slist_append (list, GINT_TO_POINTER (transaction_number));
			}
			else if (priv->search_type == 2 && search_transaction_amount_is_valide (transaction_number, text, dialog))
			{
				list = g_slist_append (list, GINT_TO_POINTER (transaction_number));
			}
		}
		tmp_list = tmp_list->next;
	}

	/* tri de la liste en fonction des dates */
	list = g_slist_sort_with_data (list, (GCompareDataFunc) search_transaction_sort_result, priv);

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
static void search_transaction_button_search_clicked (GtkButton *button,
													  SearchTransaction *dialog)
{
	const gchar *text;
	gchar *tmp_str;
	gint transaction_number;
	gint archive_number = 0;
	SearchTransactionPrivate *priv;

	priv = search_transaction_get_instance_private (dialog);

	/* regarde si on a un text avant toute chose */
	text = gtk_entry_get_text (GTK_ENTRY (priv->entry_search_str));
	if (strlen (text) == 0)
	{
		gchar *tmp_str1;
		gchar *tmp_str2;

		tmp_str1 = g_strdup (_("You must enter a search text"));
		tmp_str2 = g_strdup (_("WARNING"));

		search_transaction_warning_msg (GTK_WINDOW (dialog), tmp_str1, tmp_str2);

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
		priv->list = search_transaction_get_list (priv->account_number, text, SEARCH_TRANSACTION (dialog));
		search_active = g_slist_length (priv->list);

		if (search_active > 0)
		{
			gchar *str_number;

			/* on change eventuellement l'affichage pour une ligne */
			if (search_active > 0)
				gsb_transactions_list_change_aspect_liste (1);

			/* on affiche le résultat */
			str_number = utils_str_itoa (search_active);
			if (search_active == 1)
				tmp_str = g_strdup ("1 transaction were found");
			else
				tmp_str = g_strconcat (str_number, " transactions were found", NULL);
			g_free (str_number);

			transaction_number = GPOINTER_TO_INT (g_slist_nth_data (priv->list, search_active-1));
			if (priv->search_archive)
				archive_number = gsb_data_transaction_get_archive_number (transaction_number);
    		if (archive_number)
				search_transaction_select_archived (transaction_number, priv->account_number, archive_number);
			else
				transaction_list_select (transaction_number);
			search_active--;

			/* on desensibilise le combo other_account */
			gtk_widget_set_sensitive (priv->box_other_account, FALSE);
		}
		else
		{
			tmp_str = g_strdup ("No transaction was found");
			gtk_widget_set_sensitive (priv->box_other_account, TRUE);
			search_active = -1;
		}

		/* set info label text */
		gtk_label_set_text (GTK_LABEL (priv->label_search_info), tmp_str);
		g_free (tmp_str);
	}
	else if (search_active)
	{
		transaction_number = GPOINTER_TO_INT (g_slist_nth_data (priv->list, search_active-1));
		if (priv->search_archive)
			archive_number = gsb_data_transaction_get_archive_number (transaction_number);
		if (archive_number)
			search_transaction_select_archived (transaction_number, priv->account_number, archive_number);
		else
			transaction_list_select (transaction_number);
		search_active--;

		if (search_active == 0)
			search_active = -1;
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
static void search_transaction_checkbutton_backwards_search_toggled (GtkToggleButton *togglebutton,
																	 SearchTransactionPrivate *priv)
{
	priv->backwards_search = gtk_toggle_button_get_active (togglebutton);

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
static void search_transaction_checkbutton_delta_amount_toggled (GtkToggleButton *togglebutton,
																 SearchTransactionPrivate *priv)
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
static void search_transaction_checkbutton_ignore_case_toggled (GtkToggleButton *togglebutton,
																SearchTransactionPrivate *priv)
{
	priv->ignore_case = gtk_toggle_button_get_active (togglebutton);

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
static void search_transaction_checkbutton_search_archive_toggled (GtkToggleButton *togglebutton,
																   SearchTransactionPrivate *priv)
{
	priv->search_archive = gtk_toggle_button_get_active (togglebutton);

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
static void search_transaction_checkbutton_str_choice_toggled (GtkToggleButton *togglebutton,
															   SearchTransaction *dialog)
{
	gint button_number;
	gint state1;
	gint state2;
	SearchTransactionPrivate *priv;

	priv = search_transaction_get_instance_private (dialog);
	state1 = gtk_toggle_button_get_active (togglebutton);

	button_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (togglebutton), "pointer"));
	if (button_number == 1)
	{
		/* state1 etat bouton payee, state2 etat bouton note */
		state2 = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_note));
		if (state1 && state2)
		{
			priv->search_str_type = 3;
		}
		else if (state1 && !state2)
		{
			priv->search_str_type = 1;
		}
		else if (!state1 && state2)
		{
			priv->search_str_type = 2;
		}
		else
		{
			gchar *tmp_str1;
			gchar *tmp_str2;

			tmp_str1 = g_strdup (_("You must select at least one search element"));
        	tmp_str2 = g_strdup (_("WARNING"));

			search_transaction_warning_msg (GTK_WINDOW (dialog), tmp_str1, tmp_str2);

			g_free (tmp_str1);
			g_free (tmp_str2);
		}
	}
	else
	{
		/* state1 etat bouton note, state2 etat bouton payee */
		state2 = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_payee));
		if (state1 && state2)
		{
			priv->search_str_type = 3;
		}
		else if (state1 && !state2)
		{
			priv->search_str_type = 2;
		}
		else if (!state1 && state2)
		{
			priv->search_str_type = 1;
		}
		else
		{
			gchar *tmp_str1;
			gchar *tmp_str2;

			tmp_str1 = g_strdup (_("You must select at least one search element"));
        	tmp_str2 = g_strdup (_("WARNING"));

			search_transaction_warning_msg (GTK_WINDOW (dialog), tmp_str1, tmp_str2);

			g_free (tmp_str1);
			g_free (tmp_str2);
		}
	}

	/* on reinitialise la recherche */
	search_active = -1;
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
static gboolean search_transaction_radiobutton_press_event (GtkWidget *button,
													    	GdkEvent  *event,
													    	SearchTransactionPrivate *priv)
{
	gint button_number;

	button_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "pointer"));
	if (button_number == 1)
	{
		gtk_widget_set_sensitive (priv->box_search_amount, FALSE);
		gtk_widget_set_sensitive (priv->box_search_str, TRUE);
		gtk_widget_set_sensitive (priv->checkbutton_ignore_case, TRUE);
		priv->search_type = 1;
	}
	else
	{
		gtk_widget_set_sensitive (priv->box_search_amount, TRUE);
		gtk_widget_set_sensitive (priv->box_search_str, FALSE);
		gtk_widget_set_sensitive (priv->checkbutton_ignore_case, FALSE);
		priv->search_type = 2;
	}

	/* on reinitialise la recherche */
	search_active = -1;

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
static gboolean search_transaction_key_press_event  (GtkWidget *entry,
													 GdkEventKey *ev,
													 SearchTransaction *dialog)
{
	SearchTransactionPrivate *priv;

	priv = search_transaction_get_instance_private (dialog);
	switch (ev->keyval)
    {
		case GDK_KEY_KP_Enter :
		case GDK_KEY_Return :
			if (gtk_entry_buffer_get_length (gtk_entry_get_buffer (GTK_ENTRY (entry))))
			{
				/* on réinitialise la recherche si le text à été modifié*/
				search_active = -1;
				search_transaction_button_search_clicked (NULL, dialog);
			}
			else
			{
				gchar *tmp_str1;
				gchar *tmp_str2;

				gtk_entry_set_text (GTK_ENTRY (entry), "");
				gtk_editable_set_position (GTK_EDITABLE (entry), 0);
				gtk_widget_grab_focus (entry);

				tmp_str1 = g_strdup (_("You must enter a search text"));
				tmp_str2 = g_strdup (_("WARNING"));

				dialogue_warning_hint (tmp_str1, tmp_str2);

				g_free (tmp_str1);
				g_free (tmp_str2);

				return TRUE;
			}
			break;

		case GDK_KEY_v:         /* touche v */
			if ((ev->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
			{
				gtk_widget_set_sensitive (priv->button_search, TRUE);
			}
			break;

		default:
			if (gtk_entry_buffer_get_length (gtk_entry_get_buffer (GTK_ENTRY (entry))))
				gtk_widget_set_sensitive (priv->button_search, TRUE);
			else
				gtk_widget_set_sensitive (priv->button_search, FALSE);
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
static gboolean search_transaction_entry_lose_focus (GtkWidget *entry,
												     GdkEvent  *event,
												     SearchTransactionPrivate *priv)
{
	if (gtk_entry_buffer_get_length (gtk_entry_get_buffer (GTK_ENTRY (entry))))
	{
		const gchar *text;

		text = gtk_entry_get_text (GTK_ENTRY (priv->entry_search_str));

		/* on reinitialise eventuellement la recherche */
		if (old_recherche && strcmp (old_recherche, text))
			search_active = -1;

		gtk_widget_set_sensitive (priv->button_search, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (priv->button_search, FALSE);
	}

	return FALSE;
}

static void search_transaction_combo_other_account_changed (GtkComboBox *combo,
															SearchTransaction *dialog)
{
	GtkTreeIter iter;
	devel_debug (NULL);

	if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter))
	{
		GtkTreeModel *store;
		gint account_number;
		SearchTransactionPrivate *priv;

		store = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
        gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 1, &account_number, -1);
		printf ("account_number = %d\n", account_number);

		priv = search_transaction_get_instance_private (dialog);

		if (account_number == 0)
			return;

		/* on retablit les paramètres de lancien compte */
		gsb_transactions_list_change_aspect_liste (priv->display_nb_rows);
		if (gsb_data_account_get_r (priv->account_number) != priv->display_r)
		{
			gsb_data_account_set_r (priv->account_number, FALSE);
			gsb_menu_update_view_menu (priv->account_number);
			gsb_transactions_list_mise_a_jour_affichage_r (FALSE);
		}

		/* on change pour le nouveau compte */
	    gsb_gui_navigation_change_account (account_number);
	    gsb_account_property_fill_page ();
	    gsb_gui_navigation_set_selection (GSB_ACCOUNT_PAGE, account_number, 0);

		/* on réinitialise la recherche */
		priv->account_number = account_number;
		search_active = -1;
		search_transaction_button_search_clicked (NULL, dialog);
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
static gboolean search_transaction_combo_separator_func (GtkTreeModel *model,
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
static void search_transaction_init_combo_other_account (SearchTransactionPrivate *priv)
{
    GSList *list_tmp;
    GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeIter iter;
	gchar *msg;

    store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_BOOLEAN);

	/* set separator before current account : col 2 of model */
	gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (priv->combo_other_account),
										  (GtkTreeViewRowSeparatorFunc) search_transaction_combo_separator_func,
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
		if (account_number >= 0 && (gsb_data_account_get_kind (account_number) != GSB_TYPE_LIABILITIES))
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
static void search_transaction_setup_dialog (SearchTransaction *dialog,
											 gint transaction_number)
{
	const gchar *currency_code;
	gint currency;
	SearchTransactionPrivate *priv;

	devel_debug_int (transaction_number);
	priv = search_transaction_get_instance_private (dialog);
	priv->account_number = gsb_data_transaction_get_account_number (transaction_number);

	/* set currency */
	currency = gsb_data_account_get_currency (priv->account_number);
	currency_code = gsb_data_currency_get_nickname_or_code_iso (currency);
	gtk_label_set_text (GTK_LABEL (priv->label_devise), currency_code);

	/* set types of search */
	priv->search_type = 1;
	priv->search_str_type = 1;

	/* set number for checkbutton */
    g_object_set_data (G_OBJECT (priv->checkbutton_payee), "pointer", GINT_TO_POINTER (1));
    g_object_set_data (G_OBJECT (priv->checkbutton_note), "pointer", GINT_TO_POINTER (2));

	/* set number for radiobutton */
    g_object_set_data (G_OBJECT (priv->radiobutton_search_str), "pointer", GINT_TO_POINTER (1));
    g_object_set_data (G_OBJECT (priv->radiobutton_search_amount), "pointer", GINT_TO_POINTER (2));

	/* init ignore_case comme dans les preferences du formulaire */
	if (etat.combofix_case_sensitive)
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
	priv->backwards_search = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_backwards_search));

	/* save the number of rows */
	priv->display_nb_rows = gsb_data_account_get_nb_rows (priv->account_number);
	priv->display_r = gsb_data_account_get_r (priv->account_number);

	/* set account combo */
	search_transaction_init_combo_other_account (priv);

	/* set signals */
	g_signal_connect (G_OBJECT (priv->button_search),
					  "clicked",
					  G_CALLBACK (search_transaction_button_search_clicked),
					  dialog);

	g_signal_connect (G_OBJECT (priv->checkbutton_backwards_search),
					  "clicked",
					  G_CALLBACK (search_transaction_checkbutton_backwards_search_toggled),
					  priv);

	g_signal_connect (G_OBJECT (priv->checkbutton_delta_amount),
					  "clicked",
					  G_CALLBACK (search_transaction_checkbutton_delta_amount_toggled),
					  priv);

	g_signal_connect (G_OBJECT (priv->checkbutton_ignore_case),
					  "toggled",
					  G_CALLBACK (search_transaction_checkbutton_ignore_case_toggled),
					  priv);

	g_signal_connect (G_OBJECT (priv->checkbutton_note),
					  "toggled",
					  G_CALLBACK (search_transaction_checkbutton_str_choice_toggled),
					  dialog);

	g_signal_connect (G_OBJECT (priv->checkbutton_payee),
					  "toggled",
					  G_CALLBACK (search_transaction_checkbutton_str_choice_toggled),
					  dialog);

	g_signal_connect (G_OBJECT (priv->checkbutton_search_archive),
					  "toggled",
					  G_CALLBACK (search_transaction_checkbutton_search_archive_toggled),
					  dialog);

	g_signal_connect (G_OBJECT (priv->combo_other_account),
					  "changed",
					  G_CALLBACK (search_transaction_combo_other_account_changed),
					  dialog);

	g_signal_connect (G_OBJECT (priv->entry_search_str),
					  "focus-out-event",
					  G_CALLBACK (search_transaction_entry_lose_focus),
					  priv);
	g_signal_connect (G_OBJECT (priv->entry_search_str),
					  "key-press-event",
					  G_CALLBACK (search_transaction_key_press_event),
					  dialog);

	g_signal_connect (G_OBJECT (priv->radiobutton_search_amount),
					  "button-press-event",
					  G_CALLBACK (search_transaction_radiobutton_press_event),
					  priv);
	g_signal_connect (G_OBJECT (priv->radiobutton_search_str),
					  "button-press-event",
					  G_CALLBACK (search_transaction_radiobutton_press_event),
					  priv);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void search_transaction_init (SearchTransaction *dialog)
{
	gtk_widget_init_template (GTK_WIDGET (dialog));
}

static void search_transaction_dispose (GObject *object)
{
	SearchTransactionPrivate *priv;

	priv = search_transaction_get_instance_private (SEARCH_TRANSACTION (object));
	gsb_transactions_list_change_aspect_liste (priv->display_nb_rows);
	if (gsb_data_account_get_r (priv->account_number) != priv->display_r)
	{
		gsb_data_account_set_r (priv->account_number, FALSE);
		gsb_menu_update_view_menu (priv->account_number);
        gsb_transactions_list_mise_a_jour_affichage_r (FALSE);
	}

	G_OBJECT_CLASS (search_transaction_parent_class)->dispose (object);
}

static void search_transaction_class_init (SearchTransactionClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = search_transaction_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/ui/search_transaction.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, vbox_search_transaction);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, button_search);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, box_other_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, box_search_str);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, box_search_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, checkbutton_backwards_search);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, checkbutton_delta_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, checkbutton_ignore_case);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, checkbutton_note);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, checkbutton_payee);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, checkbutton_search_archive);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, combo_other_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, entry_search_str);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, label_devise);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, label_search_info);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, radiobutton_search_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, radiobutton_search_str);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), SearchTransaction, spinbutton_delta_amount);
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
SearchTransaction *search_transaction_new (GrisbiWin *win,
										   gint *transaction_number)
{
	gint height;
	gint width;
	GrisbiAppConf *a_conf;
	SearchTransaction *dialog;

	dialog = g_object_new (SEARCH_TRANSACTION_TYPE, "transient-for", win, NULL);

	/* set position of dialog */
	a_conf = grisbi_app_get_a_conf ();
    if (!a_conf->low_definition_screen)
	{
		gtk_window_set_gravity (GTK_WINDOW (dialog), GDK_GRAVITY_SOUTH_EAST);
		gtk_window_get_size (GTK_WINDOW (win), &width, &height);
		gtk_window_move (GTK_WINDOW (dialog), width, height/3);
	}

	search_transaction_setup_dialog (dialog, GPOINTER_TO_INT (transaction_number));

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
void search_transaction_dialog_response  (GtkDialog *dialog,
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

