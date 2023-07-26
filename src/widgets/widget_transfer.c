/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "widget_transfer.h"
#include "bet_data.h"
#include "bet_future.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_form.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_payment_method.h"
#include "gtk_combofix.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
/*END_STATIC*/

typedef struct _WidgetTransferPrivate   WidgetTransferPrivate;

struct _WidgetTransferPrivate
{
	GtkWidget *			vbox_transfer;
	GtkWidget *			button_delete_transfer;
	GtkWidget *			treeview_transfer;

	GtkWidget *			grid_card_div;
	GtkWidget *			combo_card_budget;
	GtkWidget *			combo_card_category;
	GtkWidget *			combo_card_payee;
	GtkWidget *			combo_card_payment;
	GtkWidget *			entry_card_budget;
	GtkWidget *			entry_card_category;
	GtkWidget *			entry_card_payee;
	GtkWidget *			entry_date_bascule;
	GtkWidget *			hbox_date_bascule;
	GtkWidget *			label_date_bascule;
	GtkWidget *			combo_card_choice_bascule_day;

	GtkWidget *			grid_main_div;
	GtkWidget *			checkbutton_direct_debit;
	GtkWidget *			checkbutton_replace_data;
	GtkWidget *			combo_main_budget;
	GtkWidget *			combo_main_category;
	GtkWidget *			combo_main_choice_debit_day;
	GtkWidget *			combo_main_payee;
	GtkWidget *			combo_main_payment;
	GtkWidget *			entry_date_debit;
	GtkWidget *			entry_main_budget;
	GtkWidget *			entry_main_category;
	GtkWidget *			entry_main_payee;
	GtkWidget *			hbox_date_debit;
	GtkWidget *			label_date_debit;

	/* data */
	gint				create_mode;
	gboolean			valid_card_date;
	gboolean			valid_card_payee;
	gboolean			valid_card_div;
	gboolean			valid_main_date;
	gboolean			valid_main_payee;
	gboolean			valid_main_div;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetTransfer, widget_transfer, GTK_TYPE_DIALOG)

enum BetTransferColumns {
	BET_TRANSFER_NAME,
	BET_TRANSFERT_KIND,
	BET_TRANSFER_ACCOUNT,
	BET_TRANSFERT_PARTIAL,
	BET_TRANSFERT_BACKGROUND
};

enum BetTransferFormType {
	BET_TRANSFER_PAYEE,
	BET_TRANSFER_CARD_DATE,
	BET_TRANSFER_CARD_CATEGORY,
	BET_TRANSFER_CARD_BUDGET,
	BET_TRANSFER_MAIN_DATE,
	BET_TRANSFER_MAIN_CATEGORY,
	BET_TRANSFER_MAIN_BUDGET
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * validate data before return
 *
 * \param
 *
 * \return TRUE if all data are correctly
 **/
static gboolean widget_transfer_validate_all_data (WidgetTransfer *dialog)
{
	gboolean tmp_valid = FALSE;
	gboolean valid = FALSE;
	WidgetTransferPrivate *priv;

	devel_debug (NULL);
	priv = widget_transfer_get_instance_private (dialog);

	if ((gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_replace_data)) == FALSE
		 && gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_direct_debit)) == FALSE)
		|| (priv->valid_main_payee && priv->valid_main_div))
	{
		tmp_valid = TRUE;
	}
	else
	{
		tmp_valid = FALSE;

	}

	if (priv->valid_card_date == TRUE
		&& priv->valid_card_payee == TRUE
		&& priv->valid_card_div == TRUE
		&& tmp_valid == TRUE)
		valid = TRUE;
	else
		valid = FALSE;

	return valid;
}

/**
 * calcule et affiche la date d'arrêté si la date choisie n'est pas ouvrée
 *
 * \param dialog
 * \param sensitive
 *
 * \return
 **/
static void widget_transfer_set_effective_date_bascule (WidgetTransfer *dialog,
														GDate *date)
{
	GDate *tmp_date;
	WidgetTransferPrivate *priv;
devel_debug (NULL);

	priv = widget_transfer_get_instance_private (dialog);

	gtk_widget_show (priv->hbox_date_bascule);
	tmp_date = gsb_date_get_first_banking_day_after_date (date);
	gtk_label_set_text (GTK_LABEL (priv->label_date_bascule), gsb_format_gdate (tmp_date));
}

/**
 * calcule et affiche la date d'arrêté si la date choisie n'est pas ouvrée
 *
 * \param dialog
 * \param sensitive
 *
 * \return
 **/
static void widget_transfer_set_effective_date_debit (WidgetTransfer *dialog,
													  GDate *date)
{
	GDate *tmp_date;
	WidgetTransferPrivate *priv;
devel_debug (NULL);
	priv = widget_transfer_get_instance_private (dialog);

	gtk_widget_show (priv->hbox_date_debit);
	tmp_date = gsb_date_get_first_banking_day_before_date (date);
	gtk_label_set_text (GTK_LABEL (priv->label_date_debit), gsb_format_gdate (tmp_date));
}

/**
 * sensibilise ou insensibilise les éléments de l'opération du compte principal
 *
 * \param dialog
 * \param sensitive
 *
 * \return
 **/
static void widget_transfer_sensitive_direct_debit (WidgetTransfer *dialog,
													gboolean sensitive)
{
	GtkWidget *icon_1;
	GtkWidget *icon_2;
	WidgetTransferPrivate *priv;

	priv = widget_transfer_get_instance_private (dialog);
	icon_1 = g_object_get_data (G_OBJECT (priv->combo_main_payee), "icon");
	icon_2 = g_object_get_data (G_OBJECT (priv->combo_main_category), "icon");
	if (sensitive)
	{
		gtk_widget_set_sensitive (icon_1, TRUE);
		gtk_widget_set_sensitive (icon_2, TRUE);
		gtk_widget_set_sensitive (priv->combo_main_budget, TRUE);
		gtk_widget_set_sensitive (priv->combo_main_category, TRUE);
		gtk_widget_set_sensitive (priv->combo_main_payee, TRUE);
		gtk_widget_set_sensitive (priv->combo_main_payment, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (icon_1, FALSE);
		gtk_widget_set_sensitive (icon_2, FALSE);
		gtk_widget_set_sensitive (priv->combo_main_budget, FALSE);
		gtk_widget_set_sensitive (priv->combo_main_category, FALSE);
		gtk_widget_set_sensitive (priv->combo_main_payee, FALSE);
		gtk_widget_set_sensitive (priv->combo_main_payment, FALSE);
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
static void widget_transfer_direct_debit_toggle (GtkToggleButton *button,
												 WidgetTransfer *dialog)
{
	devel_debug (NULL);
	if (gtk_toggle_button_get_active (button))
	{
		widget_transfer_sensitive_direct_debit (dialog, TRUE);
	}
	else
	{
		WidgetTransferPrivate *priv;

		priv = widget_transfer_get_instance_private (dialog);
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_replace_data)) == FALSE)
			widget_transfer_sensitive_direct_debit (dialog, FALSE);
	}

	if (widget_transfer_validate_all_data (dialog))
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
}

/**
 * sensibilise ou insensibilise la boite en fonction de l'état du bouton
 *
 * \param button
 * \param box
 *
 * \return
 **/
static void widget_transfer_replace_data_toggle (GtkToggleButton *button,
												 WidgetTransfer *dialog)
{
	devel_debug (NULL);
	if (gtk_toggle_button_get_active (button))
	{
		widget_transfer_sensitive_direct_debit (dialog, TRUE);
	}
	else
	{
		WidgetTransferPrivate *priv;

		priv = widget_transfer_get_instance_private (dialog);
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_direct_debit)) == FALSE)
			widget_transfer_sensitive_direct_debit (dialog, FALSE);
	}

	if (widget_transfer_validate_all_data (dialog))
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_transfer_combo_main_choice_debit_day_changed (GtkComboBox *combo,
															 	 WidgetTransfer *dialog)
{
	GDate *date;
	GDate *tmp_date;
	gint active;
	WidgetTransferPrivate *priv;
devel_debug (NULL);

	priv = widget_transfer_get_instance_private (dialog);
	if (!GTK_IS_DIALOG (dialog))
		return;

	active = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));
	switch (active)
	{
		case 2:
			date = gsb_calendar_entry_get_date (priv->entry_date_debit);
			widget_transfer_set_effective_date_debit (dialog, date);
			gtk_widget_set_sensitive (priv->entry_date_debit, TRUE);
			break;
		case 1:
			date = gsb_calendar_entry_get_date (priv->entry_date_debit);
			if (g_date_valid (date))
			{
				tmp_date = gsb_date_get_last_banking_day_of_month (date);
				g_date_free (date);

				if (g_date_valid (tmp_date))
				{
					gsb_calendar_entry_set_date (priv->entry_date_debit, tmp_date);
					gtk_widget_set_sensitive (priv->entry_date_debit, FALSE);
					gtk_widget_hide (priv->hbox_date_debit);
					g_date_free (tmp_date);
				}
			}

			break;
		default:
			gtk_widget_hide (priv->hbox_date_bascule);
			gtk_widget_set_sensitive (priv->entry_date_debit, TRUE);
	}

	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_transfer_card_choice_bascule_day_changed (GtkComboBox *combo,
															 WidgetTransfer *dialog)
{
	WidgetTransferPrivate *priv;
devel_debug (NULL);

	priv = widget_transfer_get_instance_private (dialog);
	if (!GTK_IS_DIALOG (dialog))
		return;

	if (gtk_combo_box_get_active (GTK_COMBO_BOX (combo)))
	{
		GDate *date;

		date = gsb_calendar_entry_get_date (priv->entry_date_bascule);
		widget_transfer_set_effective_date_bascule (dialog, date);
	}
	else
		gtk_widget_hide (priv->hbox_date_bascule);

	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_transfer_payment_method_changed (GtkComboBox *combo_box,
													WidgetTransfer *dialog)
{
	devel_debug (NULL);
	if (!GTK_IS_DIALOG (dialog))
		return;

	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
}

/**
 * sélectionne le compte concerné par la modification du solde.
 *
 * \param struct transfert
 * \param
 *
 * \return
 **/
static void widget_transfert_select_account_in_treeview (GtkWidget *tree_view,
														 TransfertData *transfert)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	gboolean valid;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));

	valid = gtk_tree_model_get_iter_first (model, &iter);
	while(valid)
	{
		gint tmp_number;
		gint type_de_compte;

		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 2, &tmp_number, 3, &type_de_compte, -1);

		if (transfert->type == type_de_compte && tmp_number == transfert->card_account_number)
		{
			gtk_tree_selection_select_iter (GTK_TREE_SELECTION (selection), &iter);
			break;
		}
		valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter);
	}
}

/**
 * called when date changed
 * check the date and set the entry red/invalid if not a good date
 *
 * \param entry
 * \param null
 *
 * \return FALSE
 * */
static void widget_transfert_calendar_entry_changed (GtkWidget *entry,
													 WidgetTransfer *dialog)
{
	GDate *date;
	WidgetTransferPrivate *priv;

	priv = widget_transfer_get_instance_private (dialog);

	devel_debug (NULL);
	/* if we are in the form and the entry is empty, do nothing
	 * because it's a special style too */
	if (gsb_form_widget_check_empty (entry))
	{
		priv->valid_card_date = FALSE;

		return;
	}

	/* if nothing in the entry, keep the normal style */
	if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
	{
		priv->valid_card_date = FALSE;

		return;
	}

	/* to check the date, we just try to see if can have a dote from the entry */
	date = gsb_date_get_last_entry_date (gtk_entry_get_text (GTK_ENTRY (entry)));

	if (date)
	{
		if (g_object_get_data (G_OBJECT (entry), "form_type") == GINT_TO_POINTER (BET_TRANSFER_CARD_DATE))
		{
			/* the date is valid, make it normal */
			if (!gtk_widget_get_sensitive (priv->combo_card_choice_bascule_day))
			{
				gsb_calendar_entry_set_color (entry, TRUE);
				gtk_widget_set_sensitive (priv->combo_card_choice_bascule_day, TRUE);
			}
			if (gtk_combo_box_get_active (GTK_COMBO_BOX (priv->combo_card_choice_bascule_day)))
			{
				widget_transfer_set_effective_date_bascule (WIDGET_TRANSFER (dialog), date);
			}
		}
		else
		{
			if (gtk_combo_box_get_active (GTK_COMBO_BOX (priv->combo_main_choice_debit_day)) == 2)
			{
				widget_transfer_set_effective_date_debit (WIDGET_TRANSFER (dialog), date);
			}
		}
		priv->valid_card_date = TRUE;
	}
	else
		priv->valid_card_date = FALSE;
}

/**
 * called when an entry get the focus, if the entry is free,
 * set it normal and erase the help content
 *
 * \param entry
 * \param ev
 * \param user_data		not used
 *
 * \return FALSE
 **/
static gboolean widget_transfert_entry_focus_in (GtkWidget *entry,
												 GdkEventFocus *ev,
												 gpointer user_data)
{
	devel_debug (NULL);
	/* the entry is a real entry */
	if (gsb_form_widget_check_empty (entry))
	{
		gtk_entry_set_text (GTK_ENTRY (entry), "");
		gsb_form_widget_set_empty (entry, FALSE);
	}

	return FALSE;
}

/**
 * callback called on focus-out on the date entry
 * complete and check the date
 *
 * \param entry		the entry which receive the signal
 * \param
 * \param
 *
 * \return true
 * */
static gboolean widget_transfert_calendar_entry_focus_out (GtkWidget *entry,
														   GdkEventFocus *event,
														   WidgetTransfer *dialog)
{
	devel_debug (NULL);
	if (widget_transfer_validate_all_data (dialog))
	{
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
	}
	else
	{
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
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
static gboolean widget_transfert_div_entry_focus_out (GtkWidget *entry,
													  GdkEventFocus *ev,
													  WidgetTransfer *dialog)
{
	GtkWidget *widget;
	const gchar *string;
	gint element_number;
	gint *ptr_origin;
	WidgetTransferPrivate *priv;

	devel_debug (NULL);
	priv = widget_transfer_get_instance_private (dialog);

	/* still not found, if change the content of the form, something come in entry
	 * which is nothing, so protect here */
	if (!GTK_IS_WIDGET (entry) || !GTK_IS_ENTRY (entry))
		return FALSE;

	/* remove the selection */
	gtk_editable_select_region (GTK_EDITABLE (entry), 0, 0);

	ptr_origin = g_object_get_data (G_OBJECT (entry), "form_type");
	element_number = GPOINTER_TO_INT (ptr_origin);
	if (element_number != BET_TRANSFER_PAYEE
		&& element_number != BET_TRANSFER_CARD_CATEGORY
		&& element_number != BET_TRANSFER_CARD_BUDGET
		&& element_number != BET_TRANSFER_MAIN_CATEGORY
		&& element_number != BET_TRANSFER_MAIN_BUDGET)
		return FALSE;

	widget = g_object_get_data (G_OBJECT (entry), "combo");

	/* sometimes the combofix popus stays showed, so remove here */
	gtk_combofix_hide_popup (GTK_COMBOFIX (widget));

	/* string will be filled only if the field is empty */
	string = NULL;
	switch (element_number)
	{
		case BET_TRANSFER_PAYEE:
			if (strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				utils_set_image_with_state (widget,TRUE);
				gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
			}
			else
			{
				string = gsb_form_widget_get_name (TRANSACTION_FORM_PARTY);
				gtk_combofix_set_text (GTK_COMBOFIX (widget), _(string));
				gsb_form_widget_set_empty (entry, TRUE);
				utils_set_image_with_state (widget, FALSE);
				gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
			}
			break;

		case BET_TRANSFER_CARD_CATEGORY:
			if (strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				utils_set_image_with_state (widget,TRUE);
				gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
			}
			else
			{
				string = gsb_form_widget_get_name (TRANSACTION_FORM_CATEGORY);
				gtk_combofix_set_text (GTK_COMBOFIX (widget), _(string));
				gsb_form_widget_set_empty (entry, TRUE);
				if (gsb_form_widget_check_empty (priv->combo_card_budget))
				{
					utils_set_image_with_state (widget, FALSE);
					gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
				}
				else
				{
					utils_set_image_with_state (widget, TRUE);
					gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
				}
			}
			break;

		case BET_TRANSFER_MAIN_CATEGORY:
			if (strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				utils_set_image_with_state (widget,TRUE);
				gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
			}
			else
			{
				string = gsb_form_widget_get_name (TRANSACTION_FORM_CATEGORY);
				gtk_combofix_set_text (GTK_COMBOFIX (widget), _(string));
				gsb_form_widget_set_empty (entry, TRUE);
				if (gsb_form_widget_check_empty (priv->combo_main_budget))
				{
					utils_set_image_with_state (widget, FALSE);
					gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
				}
				else
				{
					utils_set_image_with_state (widget, TRUE);
					gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
				}
			}
			break;

		case BET_TRANSFER_CARD_BUDGET:
			if (strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				utils_set_image_with_state (widget,TRUE);
				gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
			}
			else
			{
				string = gsb_form_widget_get_name (TRANSACTION_FORM_BUDGET);
				gtk_combofix_set_text (GTK_COMBOFIX (widget), _(string));
				gsb_form_widget_set_empty (entry, TRUE);
				if (gsb_form_widget_check_empty (priv->combo_card_category))
				{
					utils_set_image_with_state (widget, FALSE);
					gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
				}
				else
				{
					utils_set_image_with_state (widget, TRUE);
					gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
				}
			}
			break;

		case BET_TRANSFER_MAIN_BUDGET:
			if (strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				utils_set_image_with_state (widget,TRUE);
				gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
			}
			else
			{
				string = gsb_form_widget_get_name (TRANSACTION_FORM_BUDGET);
				gtk_combofix_set_text (GTK_COMBOFIX (widget), _(string));
				gsb_form_widget_set_empty (entry, TRUE);
				if (gsb_form_widget_check_empty (priv->combo_main_category))
				{
					utils_set_image_with_state (widget, FALSE);
					gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
				}
				else
				{
					utils_set_image_with_state (widget, TRUE);
					gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
				}
			}
			break;

		default :
			break;
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
static void widget_transfer_fill_list (GtkWidget *tree_view,
									   gint account_number)
{
	GtkTreeModel *model;
	GSList *tmp_list;
	gint nbre_rows = 0;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	tmp_list = gsb_data_account_get_list_accounts ();
	while (tmp_list)
	{
		gint tmp_account_number;
		gint bet_credit_card;
		GtkTreeIter iter;

		tmp_account_number = gsb_data_account_get_no_account (tmp_list->data);

		/* On passe si le compte est clos */
		if (gsb_data_account_get_closed_account (tmp_account_number))
		{
			tmp_list = tmp_list->next;
			continue;
		}

		if (tmp_account_number != account_number)
		{
			bet_credit_card = gsb_data_account_get_bet_credit_card (tmp_account_number);
			if (bet_credit_card)
			{
				KindAccount kind;
				gchar *tmp_str = NULL;

				kind = gsb_data_account_get_kind (tmp_account_number);
				switch (kind)
				{
					case GSB_TYPE_BANK:
						tmp_str = g_strdup (_("Bank account"));
						break;
					case GSB_TYPE_CASH:
						tmp_str = g_strdup (_("Cash account"));
						break;
					case GSB_TYPE_LIABILITIES:
						tmp_str = g_strdup (_("Liabilities account"));
						break;
					case GSB_TYPE_BALANCE:
						tmp_str = NULL;
						break;
					case GSB_TYPE_ASSET:
						tmp_str = NULL;
						break;
				}
				gtk_list_store_append (GTK_LIST_STORE (model), &iter);
				gtk_list_store_set (GTK_LIST_STORE (model),
									&iter,
									0, gsb_data_account_get_name (tmp_account_number),
									1, tmp_str,
									2, tmp_account_number,
									3, FALSE,
									-1);
				nbre_rows ++;
				g_free (tmp_str);
			}
		}

		tmp_list = tmp_list->next;
	}

	tmp_list = gsb_data_partial_balance_get_list ();
	while (tmp_list)
	{
		gint tmp_number;
		GtkTreeIter iter;
		const gchar *liste_cptes;
		gchar **tab;
		gint bet_credit_card = 0;
		gint i;

		tmp_number = gsb_data_partial_balance_get_number (tmp_list->data);

		liste_cptes = gsb_data_partial_balance_get_liste_cptes (tmp_number);
		tab = g_strsplit (liste_cptes, ";", 0);
		for (i = 0; i < (gint) g_strv_length (tab); i++)
		{
			gint tmp_account_number;

			tmp_account_number = utils_str_atoi (tab[i]);
			bet_credit_card = gsb_data_account_get_bet_credit_card (tmp_account_number);
		}

		g_strfreev (tab);

		if (bet_credit_card)
		{
			gtk_list_store_append (GTK_LIST_STORE (model), &iter);
			gtk_list_store_set (GTK_LIST_STORE (model),
								&iter,
								0, gsb_data_partial_balance_get_name (tmp_number),
								1, _("Partial balance"),
								2, tmp_number,
								3, TRUE,
								-1);
				nbre_rows ++;
		}

		tmp_list = tmp_list->next;
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
static void widget_transfer_setup_card_part (WidgetTransfer *dialog,
											 gint account_number)
{
	GtkWidget *combo;
	GtkWidget *icon;
	GSList *tmp_list;
	GrisbiWinEtat *w_etat;
	WidgetTransferPrivate *priv;

	devel_debug (NULL);
	priv = widget_transfer_get_instance_private (dialog);
	w_etat = grisbi_win_get_w_etat ();

	/* set entry_date_bascule */
	gsb_calendar_entry_new_from_ui (priv->entry_date_bascule, FALSE);
	g_object_set_data (G_OBJECT (priv->entry_date_bascule), "form_type", GINT_TO_POINTER (BET_TRANSFER_CARD_DATE));
	g_signal_connect (G_OBJECT (priv->entry_date_bascule),
					  "focus-in-event",
					  G_CALLBACK (widget_transfert_entry_focus_in),
					  NULL);
	g_signal_connect (G_OBJECT (priv->entry_date_bascule),
					  "focus-out-event",
					  G_CALLBACK (widget_transfert_calendar_entry_focus_out),
					  dialog);

	/* init combo_card_choice_bascule_day */
	if (priv->create_mode)
	{
		gsb_form_widget_set_empty (priv->entry_date_bascule, TRUE);
		g_signal_connect_after (G_OBJECT (priv->entry_date_bascule),
								"changed",
								G_CALLBACK (widget_transfert_calendar_entry_changed),
								dialog);

		gtk_widget_set_sensitive (priv->combo_card_choice_bascule_day, FALSE);
		g_signal_connect (G_OBJECT (priv->combo_card_choice_bascule_day),
						  "changed",
						  G_CALLBACK (widget_transfer_card_choice_bascule_day_changed),
						  dialog);
	}

	/* set card payee */
	icon = utils_get_image_with_state (GTK_MESSAGE_ERROR, FALSE, _("This field is required"), NULL);
	gtk_grid_attach (GTK_GRID (priv->grid_card_div), icon, 0, 0, 1, 1);

	tmp_list = gsb_data_payee_get_name_and_report_list ();
	combo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_payee,
											  w_etat->combofix_case_sensitive,
											  FALSE,
											  METATREE_PAYEE);
	gsb_data_payee_free_name_and_report_list (tmp_list);

	gtk_grid_attach (GTK_GRID (priv->grid_card_div), combo, 1, 0, 1, 1);
	gsb_form_widget_set_empty (combo, TRUE);
	gtk_combofix_set_text (GTK_COMBOFIX (combo), _("Payee"));

	priv->entry_card_payee = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
	g_object_set_data (G_OBJECT (priv->entry_card_payee), "combo", combo);
	g_object_set_data (G_OBJECT (priv->entry_card_payee), "form_type", GINT_TO_POINTER (BET_TRANSFER_PAYEE));
	g_object_set_data (G_OBJECT (combo), "icon", icon);
	priv->combo_card_payee = combo;

	g_signal_connect (G_OBJECT (priv->entry_card_payee),
					  "focus-in-event",
						G_CALLBACK (bet_form_entry_get_focus),
						NULL);
	g_signal_connect (G_OBJECT (priv->entry_card_payee),
						"focus-out-event",
						G_CALLBACK (widget_transfert_div_entry_focus_out),
						dialog);

	/* saisie des (sous)catégories et (sous)IB */
	icon = utils_get_image_with_state (GTK_MESSAGE_WARNING,
									  FALSE,
									  _("One of the two fields is recommended"),
									  NULL);
	gtk_grid_attach (GTK_GRID (priv->grid_card_div), icon, 0, 1, 1, 1);

	/* set categories */
	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
	combo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_CATEGORY);
	gsb_data_categorie_free_name_list (tmp_list);

	gtk_grid_attach (GTK_GRID (priv->grid_card_div), combo, 1, 1, 1, 1);
	gsb_form_widget_set_empty (combo, TRUE);
	gtk_combofix_set_text (GTK_COMBOFIX (combo), _("Categories : Sub-categories"));

	priv->entry_card_category = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
	g_object_set_data (G_OBJECT (priv->entry_card_category), "combo", combo);
	g_object_set_data (G_OBJECT (priv->entry_card_category), "form_type", GINT_TO_POINTER (BET_TRANSFER_CARD_CATEGORY));
	g_object_set_data (G_OBJECT (combo), "icon", icon);
	priv->combo_card_category = combo;

	g_signal_connect (G_OBJECT (priv->entry_card_category),
					  "focus-in-event",
					  G_CALLBACK (bet_form_entry_get_focus),
					  NULL);
	g_signal_connect (G_OBJECT (priv->entry_card_category),
					  "focus-out-event",
					  G_CALLBACK (widget_transfert_div_entry_focus_out),
					  dialog);

	/* set budget */
	tmp_list = gsb_data_budget_get_name_list (TRUE, TRUE);
	combo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_BUDGET);
	gsb_data_categorie_free_name_list (tmp_list);
	gtk_grid_attach (GTK_GRID (priv->grid_card_div), combo, 2, 1, 1, 1);
	gsb_form_widget_set_empty (combo, TRUE);
	gtk_combofix_set_text (GTK_COMBOFIX (combo), _("Budgetary line"));

	priv->entry_card_budget = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
	g_object_set_data (G_OBJECT (priv->entry_card_budget), "combo", combo);
	g_object_set_data (G_OBJECT (priv->entry_card_budget), "form_type", GINT_TO_POINTER (BET_TRANSFER_CARD_BUDGET));
	g_object_set_data (G_OBJECT (combo), "icon", icon);
	priv->combo_card_budget = combo;

	g_signal_connect (G_OBJECT (priv->entry_card_budget),
					  "focus-in-event",
					  G_CALLBACK (bet_form_entry_get_focus),
					  NULL);
	g_signal_connect (G_OBJECT (priv->entry_card_budget),
					  "focus-out-event",
					  G_CALLBACK (widget_transfert_div_entry_focus_out),
					  dialog);

	gtk_widget_show_all (priv->grid_card_div);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_transfer_setup_main_part (WidgetTransfer *dialog,
											 gint account_number)
{
	GtkWidget *combo;
	GtkWidget *icon;
	GSList *tmp_list;
	GrisbiWinEtat *w_etat;
	WidgetTransferPrivate *priv;

	devel_debug (NULL);
	priv = widget_transfer_get_instance_private (dialog);
	w_etat = grisbi_win_get_w_etat ();

	/* entry_date_debit */
	gsb_calendar_entry_new_from_ui (priv->entry_date_debit, FALSE);
	g_signal_connect (G_OBJECT (priv->entry_date_debit),
					  "focus-out-event",
					  G_CALLBACK (widget_transfert_calendar_entry_focus_out),
					  dialog);

	/* création de la boite de sélection du tiers */
	icon = utils_get_image_with_state (GTK_MESSAGE_ERROR, FALSE, _("This field is required"), NULL);
	gtk_grid_attach (GTK_GRID (priv->grid_main_div), icon, 0, 2, 1, 1);

	tmp_list = gsb_data_payee_get_name_and_report_list ();
	combo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_payee,
											  w_etat->combofix_case_sensitive,
											  FALSE,
											  METATREE_PAYEE);
	gsb_data_payee_free_name_and_report_list (tmp_list);

	gtk_grid_attach (GTK_GRID (priv->grid_main_div), combo, 1, 2, 1, 1);
	gsb_form_widget_set_empty (combo, TRUE);
	gtk_combofix_set_text (GTK_COMBOFIX (combo), _("Payee"));

	priv->entry_main_payee = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
	g_object_set_data (G_OBJECT (priv->entry_main_payee), "combo", combo);
	g_object_set_data (G_OBJECT (priv->entry_main_payee), "form_type", GINT_TO_POINTER (BET_TRANSFER_PAYEE));
	g_object_set_data (G_OBJECT (combo), "icon", icon);
	priv->combo_main_payee = combo;

	g_signal_connect (G_OBJECT (priv->entry_main_payee),
					  "focus-in-event",
					  G_CALLBACK (bet_form_entry_get_focus),
					  NULL);
	g_signal_connect (G_OBJECT (priv->entry_main_payee),
					  "focus-out-event",
					  G_CALLBACK (widget_transfert_div_entry_focus_out),
					  dialog);

	/* Création du mode de payement */
	gsb_payment_method_create_combo_list (priv->combo_main_payment,
										  GSB_PAYMENT_DEBIT,
										  account_number,
										  0,
										  FALSE);
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_main_payment), 0);
	gtk_widget_set_tooltip_text (GTK_WIDGET (priv->combo_main_payment), _("Choose the method of payment"));

	/* saisie des (sous)catégories et (sous)IB */
	icon = utils_get_image_with_state (GTK_MESSAGE_ERROR, FALSE, _("One of the two fields is required"), NULL);
	gtk_grid_attach (GTK_GRID (priv->grid_main_div), icon, 0, 3, 1, 1);

	/* set categories */
	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
	combo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_CATEGORY);
	gsb_data_categorie_free_name_list (tmp_list);

	gtk_grid_attach (GTK_GRID (priv->grid_main_div), combo, 1, 3, 1, 1);
	gsb_form_widget_set_empty (combo, TRUE);
	gtk_combofix_set_text (GTK_COMBOFIX (combo), _("Categories : Sub-categories"));

	priv->entry_main_category = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
	g_object_set_data (G_OBJECT (priv->entry_main_category), "combo", combo);
	g_object_set_data (G_OBJECT (priv->entry_main_category), "form_type", GINT_TO_POINTER (BET_TRANSFER_MAIN_CATEGORY));
	g_object_set_data (G_OBJECT (combo), "icon", icon);
	priv->combo_main_category = combo;

	g_signal_connect (G_OBJECT (priv->entry_main_category),
					  "focus-in-event",
					  G_CALLBACK (bet_form_entry_get_focus),
					  NULL);
	g_signal_connect (G_OBJECT (priv->entry_main_category),
					  "focus-out-event",
					  G_CALLBACK (widget_transfert_div_entry_focus_out),
					  dialog);

	/* set budget */
	tmp_list = gsb_data_budget_get_name_list (TRUE, TRUE);
	combo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_BUDGET);
	gsb_data_categorie_free_name_list (tmp_list);

	gtk_grid_attach (GTK_GRID (priv->grid_main_div), combo, 2, 3, 1, 1);
	gsb_form_widget_set_empty (combo, TRUE);
	gtk_combofix_set_text (GTK_COMBOFIX (combo), _("Budgetary line"));

	priv->entry_main_budget = gtk_combofix_get_entry (GTK_COMBOFIX (combo));
	g_object_set_data (G_OBJECT (priv->entry_main_budget), "combo", combo);
	g_object_set_data (G_OBJECT (priv->entry_main_budget), "form_type", GINT_TO_POINTER (BET_TRANSFER_MAIN_BUDGET));
	g_object_set_data (G_OBJECT (combo), "icon", icon);
	priv->combo_main_budget = combo;

	g_signal_connect (G_OBJECT (priv->entry_main_budget),
					  "focus-in-event",
					  G_CALLBACK (bet_form_entry_get_focus),
					  NULL);
	g_signal_connect (G_OBJECT (priv->entry_main_budget),
					  "focus-out-event",
					  G_CALLBACK (widget_transfert_div_entry_focus_out),
					  dialog);

	/* sensitive main part */
	widget_transfer_sensitive_direct_debit (dialog, FALSE);

	/* set checkbuttons */
	if (priv->create_mode)
	{
		/* set checkbutton_direct_debit */
		g_signal_connect (G_OBJECT (priv->checkbutton_direct_debit),
						  "toggled",
						  G_CALLBACK (widget_transfer_direct_debit_toggle),
						  dialog);
		/* checkbutton_replace_data */
		g_signal_connect (G_OBJECT (priv->checkbutton_replace_data),
						  "toggled",
						  G_CALLBACK (widget_transfer_replace_data_toggle),
						  dialog);
	}

	gtk_widget_show_all (priv->grid_main_div);
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean widget_transfer_selection_changed (GtkTreeSelection *selection,
												   GtkTreeModel *model,
												   GtkTreePath *path,
												   gboolean path_currently_selected,
												   WidgetTransfer *dialog)
{
	WidgetTransferPrivate *priv;

	devel_debug (NULL);
	priv = widget_transfer_get_instance_private (dialog);

	if (!GTK_IS_DIALOG (dialog))
		return FALSE;

	if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), NULL, NULL))
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);
	else
		gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);

	if (priv->create_mode)
	{
		GtkTreeIter iter;
		gint tmp_number;
		gint type_de_compte;

		gtk_tree_model_get_iter (model, &iter, path);
		gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, 2, &tmp_number, 3, &type_de_compte, -1);

		if (type_de_compte)
		{
			gtk_widget_hide (priv->combo_card_payment);
		}
		else
		{
			gsb_payment_method_create_combo_list (priv->combo_card_payment,
												  GSB_PAYMENT_CREDIT,
												  tmp_number,
												  0,
												  FALSE);
			gtk_widget_set_tooltip_text (GTK_WIDGET (priv->combo_card_payment), _("Choose the method of payment"));
			gsb_payment_method_set_payment_position (priv->combo_card_payment,
													 gsb_data_account_get_default_credit (tmp_number));

			if (!GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (priv->combo_card_payment), "handler_id")))
			{
				gulong handler_id;

				handler_id = g_signal_connect (G_OBJECT (priv->combo_card_payment),
											   "changed",
											   G_CALLBACK (widget_transfer_payment_method_changed),
											   dialog);
				g_object_set_data (G_OBJECT (priv->combo_card_payment), "handler_id", GUINT_TO_POINTER (handler_id));
			}

			gtk_widget_show (priv->combo_card_payment);
		}

		return TRUE;
	}
	else
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
static void widget_transfer_setup_treeview (WidgetTransfer *dialog,
											gint account_number)
{
	GtkListStore *model;
	GtkTreeViewColumn *column;
	GtkCellRenderer *cell;
	GtkTreeSelection *selection;
	WidgetTransferPrivate *priv;

	devel_debug (NULL);
	priv = widget_transfer_get_instance_private (dialog);

	/* create the model */
	model = gtk_list_store_new (5,
								G_TYPE_STRING,		/* account name */
								G_TYPE_STRING,		/* account kind : Cash account or Partial balance */
								G_TYPE_INT,			/* account_number or partial_balance_number */
								G_TYPE_BOOLEAN,		/* FALSE = account, TRUE = partial balance */
								GDK_TYPE_RGBA);


	/* create the treeview */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_transfer), GTK_TREE_MODEL (model));
	g_object_unref (model);

	/* Account_name */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Name"),
													   cell,
													   "text", BET_TRANSFER_NAME,
													   "cell-background-rgba", BET_TRANSFERT_BACKGROUND,
													   NULL);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
	gtk_tree_view_column_set_sort_column_id (column, 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_transfer), column);

	/* type of account */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Type"),
													   cell,
													   "text", BET_TRANSFERT_KIND,
													   "cell-background-rgba", BET_TRANSFERT_BACKGROUND,
													   NULL);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
	gtk_tree_view_column_set_sort_column_id (column, 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_transfer), column);

	/* fill the list */
	widget_transfer_fill_list (GTK_WIDGET (priv->treeview_transfer), account_number);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_transfer));
	gtk_tree_selection_set_select_function (selection,
											(GtkTreeSelectionFunc) widget_transfer_selection_changed,
											dialog,
											NULL);

	/*colorize the tree_view */
	utils_set_list_store_background_color (priv->treeview_transfer, BET_TRANSFERT_BACKGROUND);
}

/**
 * Création de la page de gestion des transfer
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_transfer_setup_page (WidgetTransfer *dialog,
										gint account_number,
										gboolean create_mode)
{
	WidgetTransferPrivate *priv;

	devel_debug (NULL);
	priv = widget_transfer_get_instance_private (dialog);
	priv->create_mode = create_mode;

	if (create_mode)
		gtk_widget_hide (priv->button_delete_transfer);

	widget_transfer_setup_treeview (dialog, account_number);
	widget_transfer_setup_card_part (dialog, account_number);
	widget_transfer_setup_main_part (dialog, account_number);

	gtk_widget_show (GTK_WIDGET (dialog));
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_transfer_init (WidgetTransfer *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void widget_transfer_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_transfer_parent_class)->dispose (object);
}

static void widget_transfer_class_init (WidgetTransferClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_transfer_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_transfer.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, vbox_transfer);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, button_delete_transfer);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, treeview_transfer);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, grid_card_div);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, combo_card_payment);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, entry_date_bascule);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, hbox_date_bascule);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, label_date_bascule);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, combo_card_choice_bascule_day);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, grid_main_div);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, checkbutton_direct_debit);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, checkbutton_replace_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, combo_main_payment);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, entry_date_debit);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, hbox_date_debit);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, label_date_debit);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetTransfer, combo_main_choice_debit_day);
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
WidgetTransfer *widget_transfer_new (gint account_number,
									 gboolean create_mode)
{
	WidgetTransfer *dialog;

	dialog =  g_object_new (WIDGET_TRANSFER_TYPE, NULL);
	widget_transfer_setup_page (dialog, account_number, create_mode);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (grisbi_app_get_active_window (NULL)));

	return dialog;
}

/**
 * remplit la boite de dialogue avec les données du solde de compte
 *
 * \param account_number
 * \param number			numéro du solde partiel
 * \param
 *
 * \return					TRUE if OK or FALSE
 **/
void widget_transfer_fill_data_from_line (GtkWidget *dialog,
										  gint account_number,
										  gint number)
{
	GHashTable *transfert_list;
	gchar *key;
	gchar *number_str, *account_number_str;
	TransfertData *transfert;
	WidgetTransferPrivate *priv;

	devel_debug (NULL);
	priv = widget_transfer_get_instance_private (WIDGET_TRANSFER (dialog));

	number_str = utils_str_itoa (number);
	if (account_number == 0)
		key = g_strconcat ("0:", number_str, NULL);
	else
	{
		account_number_str = utils_str_itoa (account_number);
		key = g_strconcat (account_number_str, ":", number_str, NULL);
		g_free(account_number_str);
	}
	g_free(number_str);

	transfert_list = bet_data_transfert_get_list ();

	transfert = g_hash_table_lookup (transfert_list, key);
	if (transfert == NULL)
		return;

	/* selectionne le compte concerné */
	priv->create_mode = TRUE;
	widget_transfert_select_account_in_treeview (priv->treeview_transfer, transfert);
	priv->create_mode = FALSE;

	/* debit card part*/

	/* set date bascule and combo_card_choice_bascule_day */
	if (transfert->date_bascule)
	{
		gsb_calendar_entry_set_date (priv->entry_date_bascule, transfert->date_bascule);
		gsb_form_widget_set_empty (priv->entry_date_bascule, FALSE);
		gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_card_choice_bascule_day),
								  transfert->card_choice_bascule_day);

		if (transfert->card_choice_bascule_day)
		{
			widget_transfer_set_effective_date_bascule (WIDGET_TRANSFER (dialog), transfert->date_bascule);
		}
	}
	else
		gsb_form_widget_set_empty (priv->entry_date_bascule, TRUE);

	g_signal_connect_after (G_OBJECT (priv->entry_date_bascule),
							"changed",
							G_CALLBACK (widget_transfert_calendar_entry_changed),
							dialog);

	g_signal_connect (G_OBJECT (priv->combo_card_choice_bascule_day),
					  "changed",
					  G_CALLBACK (widget_transfer_card_choice_bascule_day_changed),
					  dialog);

	/* set card_payee */
	if (transfert->card_payee_number > 0)
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_card_payee),
							   gsb_data_payee_get_name (transfert->card_payee_number, FALSE));

		gsb_form_widget_set_empty (priv->combo_card_payee, FALSE);
		gtk_editable_set_position (GTK_EDITABLE (gtk_combofix_get_entry
												 (GTK_COMBOFIX (priv->combo_card_payee))), 0);

		/* on actualise l'état de l'icone associée */
		utils_set_image_with_state (priv->combo_card_payee, TRUE);
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_card_payee), _("Payee"));
		gsb_form_widget_set_empty (priv->combo_card_payee, TRUE);

		/* on actualise l'état de l'icone associée */
		utils_set_image_with_state (priv->combo_card_payee, FALSE);
	}

	/* set card_payment_method if type = account */
	if (transfert->type == 0)
	{
		gsb_payment_method_create_combo_list (priv->combo_card_payment,
											  GSB_PAYMENT_CREDIT,
											  transfert->card_account_number,
											  0,
											  FALSE);
		gtk_widget_set_tooltip_text (GTK_WIDGET (priv->combo_card_payment), _("Choose the method of payment"));
		gsb_payment_method_set_payment_position (priv->combo_card_payment, transfert->card_payment_number);
		g_signal_connect (G_OBJECT (priv->combo_card_payment),
						  "changed",
						  G_CALLBACK (widget_transfer_payment_method_changed),
						  dialog);
	}
	else
	{
		gtk_widget_set_sensitive (priv->combo_card_payment, FALSE);
	}

	/* set card_category */
	if (transfert->card_category_number > 0)
	{
		gchar *tmp_str = gsb_data_category_get_name (transfert->card_category_number,
											  transfert->card_sub_category_number,
											  NULL);
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_card_category), tmp_str);
		g_free(tmp_str);
		gsb_form_widget_set_empty (priv->combo_card_category, FALSE);
		gtk_editable_set_position (GTK_EDITABLE (gtk_combofix_get_entry
												 (GTK_COMBOFIX (priv->combo_card_category))), 0);
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_card_category), _("Categories : Sub-categories"));
		gsb_form_widget_set_empty (priv->combo_card_category, TRUE);
	}

	/* set card_budget */
	if (transfert->card_budgetary_number > 0)
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_card_budget),
							   gsb_data_budget_get_name (transfert->card_budgetary_number,
														 transfert->card_sub_budgetary_number,
														 NULL));
		gsb_form_widget_set_empty (priv->combo_card_budget, FALSE);
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_card_budget), _("Budgetary line"));
		gsb_form_widget_set_empty (priv->combo_card_budget, TRUE);
	}

	if (transfert->card_category_number > 0 || transfert->card_budgetary_number > 0)
		utils_set_image_with_state (priv->combo_card_budget, TRUE);
	else
		utils_set_image_with_state (priv->combo_card_budget, FALSE);

	/* main account part */

	/* sensitive main part */
	if (transfert->direct_debit || transfert->replace_transaction)
		widget_transfer_sensitive_direct_debit (WIDGET_TRANSFER (dialog), TRUE);

	/* date debit */
	gsb_calendar_entry_set_date (priv->entry_date_debit, transfert->date_debit);
	gsb_form_widget_set_empty (priv->entry_date_debit, FALSE);

	/* set main_previous_day and init hbox_debit_day */
	g_signal_connect (G_OBJECT (priv->combo_main_choice_debit_day),
					  "changed",
					  G_CALLBACK (widget_transfer_combo_main_choice_debit_day_changed),
					  dialog);
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_main_choice_debit_day),
							  transfert->main_choice_debit_day);

	/* set main_payee */
	if (transfert->main_payee_number > 0)
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_main_payee),
							   gsb_data_payee_get_name (transfert->main_payee_number, FALSE));

		gsb_form_widget_set_empty (priv->combo_main_payee, FALSE);
		gtk_editable_set_position (GTK_EDITABLE (gtk_combofix_get_entry
												 (GTK_COMBOFIX (priv->combo_main_payee))), 0);

		 /* on actualise l'état de l'icone associée */
		utils_set_image_with_state (priv->combo_main_payee, TRUE);
		priv->valid_main_payee = TRUE;
   }
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_main_payee), _("Payee"));
		gsb_form_widget_set_empty (priv->combo_main_payee, TRUE);
		priv->valid_main_payee = FALSE;
	}

	/* set main_payment_method */
	gsb_payment_method_set_payment_position (priv->combo_main_payment, transfert->main_payment_number);

	g_signal_connect (G_OBJECT (priv->combo_main_payment),
					  "changed",
					  G_CALLBACK (widget_transfer_payment_method_changed),
					  dialog);

	/* set main category */
	if (transfert->main_category_number > 0)
	{
		gchar *tmp_str = gsb_data_category_get_name (transfert->main_category_number,
											  transfert->main_sub_category_number,
											  NULL);
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_main_category), tmp_str);
		g_free(tmp_str);
		gsb_form_widget_set_empty (priv->combo_main_category, FALSE);
		gtk_editable_set_position (GTK_EDITABLE (gtk_combofix_get_entry
												 (GTK_COMBOFIX (priv->combo_main_category))), 0);
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_main_category), _("Categories : Sub-categories"));
		gsb_form_widget_set_empty (priv->combo_main_category, TRUE);

		/* on actualise l'état de l'icone associée */
		utils_set_image_with_state (priv->combo_main_category, FALSE);
	}

	/* main budget */
	if (transfert->main_budgetary_number > 0)
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_main_budget),
							   gsb_data_budget_get_name (transfert->main_budgetary_number,
														 transfert->main_sub_budgetary_number,
														 NULL));
		gsb_form_widget_set_empty (priv->combo_main_budget, FALSE);
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (priv->combo_main_budget), _("Budgetary line"));
		gsb_form_widget_set_empty (priv->combo_main_budget, TRUE);
	}

	if (transfert->main_category_number > 0 || transfert->main_budgetary_number > 0)
	{
		utils_set_image_with_state (priv->combo_main_budget, TRUE);
		priv->valid_main_div = TRUE;
	}
	else
	{
		utils_set_image_with_state (priv->combo_main_budget, FALSE);
		priv->valid_main_div = FALSE;
	}

	/* set checkbutton_direct_debit */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_direct_debit), transfert->direct_debit);
	g_signal_connect (G_OBJECT (priv->checkbutton_direct_debit),
					  "toggled",
					  G_CALLBACK (widget_transfer_direct_debit_toggle),
					  dialog);

	/* checkbutton_replace_data */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_replace_data),
								  transfert->replace_transaction);
	g_signal_connect (G_OBJECT (priv->checkbutton_replace_data),
					  "toggled",
					  G_CALLBACK (widget_transfer_replace_data_toggle),
					  dialog);

	gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, FALSE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void widget_transfer_set_date_debit (GtkWidget *dialog,
									 GDate *date_debit)
{
	WidgetTransferPrivate *priv;

	devel_debug (NULL);
	priv = widget_transfer_get_instance_private (WIDGET_TRANSFER (dialog));

	gsb_calendar_entry_set_date (priv->entry_date_debit, date_debit);
	gsb_form_widget_set_empty (priv->entry_date_debit, FALSE);

	if (priv->create_mode)
		g_signal_connect (G_OBJECT (priv->combo_main_choice_debit_day),
						  "changed",
						  G_CALLBACK (widget_transfer_combo_main_choice_debit_day_changed),
						  dialog);
}

/**
 * récupère les données du formulaire dialog
 *
 * \param   structure transfert
 * \param   dialog
 *
 * \return TRUE if OK FALSE if error
 **/
TransfertData *widget_transfer_take_data (GtkWidget *dialog,
										  gint account_number,
										  gint number)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreeSelection *selection;
	gint replace_account;
	gint type;
	TransfertData *transfert;
	WidgetTransferPrivate *priv;

	devel_debug (NULL);
	priv = widget_transfer_get_instance_private (WIDGET_TRANSFER (dialog));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_transfer));
	if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), &model, &iter))
		return NULL;

	transfert = struct_initialise_bet_transfert ();
	transfert->main_account_number = account_number;
	transfert->number = number;

	gtk_tree_model_get (GTK_TREE_MODEL (model),
						&iter,
						BET_TRANSFER_ACCOUNT, &replace_account,
						BET_TRANSFERT_PARTIAL, &type,
						-1);
	transfert->card_account_number = replace_account;
	transfert->type = type;

	/* Account with deferred debit card */
	if (gsb_form_widget_check_empty (priv->entry_date_bascule) == FALSE)
	{
		transfert->date_bascule = gsb_calendar_entry_get_date (priv->entry_date_bascule);
		if (transfert->date_bascule == NULL)
		{
			struct_free_bet_transfert (transfert);

			return NULL;
		}
	}
	else
	{
		struct_free_bet_transfert (transfert);

		return NULL;
	}

	/* get combo_card_choice_bascule_day */
	transfert->card_choice_bascule_day = gtk_combo_box_get_active (GTK_COMBO_BOX
																   (priv->combo_card_choice_bascule_day));

	if (gsb_form_widget_check_empty (priv->combo_card_payee) == FALSE)
	{
		bet_utils_get_payee_data (priv->combo_card_payee, 2, (gpointer) transfert);
		if (transfert->card_payee_number == 0)
		{
			struct_free_bet_transfert (transfert);

			return NULL;
		}
	}
	else
	{
		struct_free_bet_transfert (transfert);

		return NULL;
	}

	bet_utils_get_payment_data (priv->combo_card_payment, 2, (gpointer) transfert);

	if (gsb_form_widget_check_empty (priv->combo_card_category) == FALSE)
	{
		bet_utils_get_category_data (priv->combo_card_category, 2, (gpointer) transfert);
	}

	if (gsb_form_widget_check_empty(priv->combo_card_budget) == FALSE)
	{
		bet_utils_get_budget_data (priv->combo_card_budget, 2, (gpointer) transfert);
	}

	/* Main account */
	if (gsb_form_widget_check_empty (priv->entry_date_debit) == FALSE)
	{
		transfert->date_debit = gsb_calendar_entry_get_date (priv->entry_date_debit);
		if (transfert->date_debit == NULL)
		{
			struct_free_bet_transfert (transfert);

			return NULL;
		}
	}
	else
	{
		struct_free_bet_transfert (transfert);

		return NULL;
	}
	transfert->main_choice_debit_day = gtk_combo_box_get_active (GTK_COMBO_BOX
																 (priv->combo_main_choice_debit_day));

	transfert->replace_transaction = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
																   (priv->checkbutton_replace_data));

	transfert->direct_debit = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON
															(priv->checkbutton_direct_debit));

	if (transfert->replace_transaction || transfert->direct_debit)
	{
		gboolean empty = TRUE;

		if (gsb_form_widget_check_empty (priv->combo_main_payee) == FALSE)
		{
			bet_utils_get_payee_data (priv->combo_main_payee, 1, (gpointer) transfert);
			if (transfert->main_payee_number > 0)
				empty = FALSE;
		}

		bet_utils_get_payment_data (priv->combo_main_payment, 1, (gpointer) transfert);

		if (gsb_form_widget_check_empty (priv->combo_main_category) == FALSE)
		{
			bet_utils_get_category_data (priv->combo_main_category, 1, (gpointer) transfert);
			empty = FALSE;
		}

		if (gsb_form_widget_check_empty(priv->combo_main_budget) == FALSE)
		{
			bet_utils_get_budget_data (priv->combo_main_budget, 1, (gpointer) transfert);
			empty = FALSE;
		}

		if (empty)
		{
			struct_free_bet_transfert (transfert);

			return NULL;
		}
	}

	return transfert;
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

