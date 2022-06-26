/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2022 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org/                                              */
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
#include "prefs_page_bet_account.h"
#include "bet_data.h"
#include "dialog.h"
#include "gsb_data_account.h"
#include "bet_data_finance.h"
#include "bet_hist.h"
#include "gsb_account.h"
#include "gsb_automem.h"
#include "gsb_calendar_entry.h"
#include "gsb_fyear.h"
#include "navigation.h"
#include "widget_loan.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "utils_widgets.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean prefs_page_bet_account_changed (GtkWidget *combo,
															PrefsPageBetAccount *page);
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPageBetAccountPrivate   PrefsPageBetAccountPrivate;

struct _PrefsPageBetAccountPrivate
{
	GtkWidget *			vbox_bet_account;

	GtkWidget *			notebook_bet_account;
	GtkWidget *			combo_bet_account;
	GtkWidget *			hbox_bet_select_account;
	GtkWidget *			checkbutton_use_bet_module;
	GtkWidget *			hbox_bet_credit_card;
	GtkWidget *			checkbutton_bet_credit_card;
	GtkWidget *			vbox_bank_cash_account;

	GtkWidget *			hbox_forecast_data;
	GtkWidget *			vbox_forecast_data;
	GtkWidget *			radiobutton_source_transaction0;
	GtkWidget *			radiobutton_source_transaction1;
	GtkWidget *			radiobutton_source_transaction2;
	GtkWidget *			radiobutton_source_transaction3;
	GtkWidget *			radiobutton_source_scheduled0;
	GtkWidget *			radiobutton_source_scheduled1;
	GtkWidget *			radiobutton_source_scheduled2;
	GtkWidget *			radiobutton_source_scheduled3;
	GtkWidget *			radiobutton_source_futur0;
	GtkWidget *			radiobutton_source_futur1;
	GtkWidget *			radiobutton_source_futur2;
	GtkWidget *			radiobutton_source_futur3;

	GtkWidget *			hbox_hist_data;
	GtkWidget *			vbox_hist_data;
	GtkWidget *			vbox_hist_card_account_use_data;
	GtkWidget *			label_hist_card_account_use_data;
	GtkWidget *			vbox_hist_main_account_use_data;
	GtkWidget *			checkbutton_hist_main_account_use_data;
	GtkWidget *			label_hist_main_account_use_data;
	GtkWidget *			vbox_liabilities_account;
	GtkWidget *			vbox_credit_data;
	GtkWidget *			notebook_credit_data;
	GtkWidget *			vbox_loan_data;
	GtkWidget *			grid_loan_data;
	GtkWidget *			spinbutton_rate_interest;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPageBetAccount, prefs_page_bet_account, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_bet_account_set_label_bet_hist_main_account_use_data (gint number,
																			 PrefsPageBetAccountPrivate *priv)
{
	gchar *tmp_str;

	if (number == 1)
		tmp_str = g_strdup (_("(These data are aggregated with those "
							  "of a deferred debit card account)"));
	else
		tmp_str = g_strdup_printf (_("(These data are aggregated with those "
									 "of %d deferred debit card accounts)"),
								   number);

	gtk_label_set_text (GTK_LABEL (priv->label_hist_main_account_use_data), tmp_str);
	gtk_widget_show (priv->vbox_hist_main_account_use_data);

	g_free (tmp_str);
}

/**
 * called for a change in automem_radiobutton3
 *
 * \param checkbutton	the button which changed
 * \param value_ptr		a gint* which is the value to set in the memory (0, 1 or 2)
 *
 * \return FALSE
 **/
static void prefs_page_bet_account_select_label_toggled (GtkWidget *togglebutton,
														 gpointer data)
{
	gint value;
	gint origine;
	gint account_number;

	devel_debug (NULL);

	/* we are on the active button, so save the value for it */
	value = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (togglebutton), "pointer"));
	origine = GPOINTER_TO_INT (data);
	account_number = gsb_account_get_account_from_combo ();

	gsb_data_account_set_bet_select_label (account_number, origine, value);

	utils_prefs_gsb_file_set_modified ();

	gsb_data_account_set_bet_maj (account_number, BET_MAJ_ESTIMATE);
	bet_data_update_bet_module (account_number, -1);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void prefs_page_bet_account_init_select_labels_widget (PrefsPageBetAccountPrivate *priv)
{
	gint origine;
	gint select;

	/* set labels for transactions */
	origine = SPP_ORIGIN_TRANSACTION;
	select = gsb_data_account_get_bet_select_label (gsb_account_get_account_from_combo (), origine);
	switch (select)
	{
		case 0:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_transaction0), TRUE);
			break;

		case 1:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_transaction1), TRUE);
			break;

		case 2:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_transaction2), TRUE);
			break;

		case 3:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_transaction3), TRUE);
			break;
	}

	/* set data for each widget */
	g_object_set_data (G_OBJECT (priv->radiobutton_source_transaction0), "pointer", GINT_TO_POINTER (0));
	g_object_set_data (G_OBJECT (priv->radiobutton_source_transaction1), "pointer", GINT_TO_POINTER (1));
	g_object_set_data (G_OBJECT (priv->radiobutton_source_transaction2), "pointer", GINT_TO_POINTER (2));
	g_object_set_data (G_OBJECT (priv->radiobutton_source_transaction3), "pointer", GINT_TO_POINTER (3));

	/* Connect signal */
	g_signal_connect (G_OBJECT (priv->radiobutton_source_transaction0),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_TRANSACTION));

	g_signal_connect (G_OBJECT (priv->radiobutton_source_transaction1),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_TRANSACTION));

	g_signal_connect (G_OBJECT (priv->radiobutton_source_transaction2),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_TRANSACTION));

	g_signal_connect (G_OBJECT (priv->radiobutton_source_transaction3),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_TRANSACTION));

	/* set labels for scheduled */
	origine = SPP_ORIGIN_SCHEDULED;
	select = gsb_data_account_get_bet_select_label (gsb_account_get_account_from_combo (), origine);
	switch (select)
	{
		case 0:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_scheduled0), TRUE);
			break;

		case 1:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_scheduled1), TRUE);
			break;

		case 2:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_scheduled2), TRUE);
			break;

		case 3:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_scheduled3), TRUE);
			break;
	}

	/* set data for each widget */
	g_object_set_data (G_OBJECT (priv->radiobutton_source_scheduled0), "pointer", GINT_TO_POINTER (0));
	g_object_set_data (G_OBJECT (priv->radiobutton_source_scheduled1), "pointer", GINT_TO_POINTER (1));
	g_object_set_data (G_OBJECT (priv->radiobutton_source_scheduled2), "pointer", GINT_TO_POINTER (2));
	g_object_set_data (G_OBJECT (priv->radiobutton_source_scheduled3), "pointer", GINT_TO_POINTER (3));

	/* Connect signal */
	g_signal_connect (G_OBJECT (priv->radiobutton_source_scheduled0),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_SCHEDULED));

	g_signal_connect (G_OBJECT (priv->radiobutton_source_scheduled1),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_SCHEDULED));

	g_signal_connect (G_OBJECT (priv->radiobutton_source_scheduled2),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_SCHEDULED));
	g_signal_connect (G_OBJECT (priv->radiobutton_source_scheduled3),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_SCHEDULED));

	/* set labels for futur data */
	origine = SPP_ORIGIN_FUTURE;
	select = gsb_data_account_get_bet_select_label (gsb_account_get_account_from_combo (), origine);
	switch (select)
	{
		case 0:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_futur0), TRUE);
			break;

		case 1:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_futur1), TRUE);
			break;

		case 2:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_futur2), TRUE);
			break;

		case 3:
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_source_futur3), TRUE);
			break;
	}

	/* set data for each widget */
	g_object_set_data (G_OBJECT (priv->radiobutton_source_futur0), "pointer", GINT_TO_POINTER (0));
	g_object_set_data (G_OBJECT (priv->radiobutton_source_futur1), "pointer", GINT_TO_POINTER (1));
	g_object_set_data (G_OBJECT (priv->radiobutton_source_futur2), "pointer", GINT_TO_POINTER (2));
	g_object_set_data (G_OBJECT (priv->radiobutton_source_futur3), "pointer", GINT_TO_POINTER (3));

	/* Connect signal */
	g_signal_connect (G_OBJECT (priv->radiobutton_source_futur0),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_FUTURE));

	g_signal_connect (G_OBJECT (priv->radiobutton_source_futur1),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_FUTURE));

	g_signal_connect (G_OBJECT (priv->radiobutton_source_futur2),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_FUTURE));
	g_signal_connect (G_OBJECT (priv->radiobutton_source_futur3),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_label_toggled),
					  GINT_TO_POINTER (SPP_ORIGIN_FUTURE));
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_bet_account_initialise_duration_widget (gint account_number,
                                                               GtkWidget *notebook)
{
    GtkWidget *widget = NULL;
    GtkWidget *button = NULL;
    GtkWidget *account_page;
    gpointer ptr = NULL;
    gint param;
    gint months;

    account_page = grisbi_win_get_account_page ();

    param = gsb_data_account_get_bet_spin_range (account_number);
    months = gsb_data_account_get_bet_months (account_number);
    button = g_object_get_data (G_OBJECT (account_page), "bet_config_account_spin_button");

    if (button && G_IS_OBJECT (button))
    {
        ptr = g_object_get_data (G_OBJECT (button), "pointer");
        g_signal_handlers_block_by_func (G_OBJECT (button),
                                         G_CALLBACK (utils_widget_duration_number_changed),
                                         ptr);
    }

    if (param == 0)
    {
        widget = g_object_get_data (G_OBJECT (account_page), "bet_config_account_previous");
        if (widget && G_IS_OBJECT (widget))
        {
            g_signal_handlers_block_by_func (G_OBJECT (widget),
                                             G_CALLBACK (utils_widget_duration_button_released),
                                             button);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);
        }
        if (button)
        {
            gtk_spin_button_set_range (GTK_SPIN_BUTTON (button), 1.0, PREV_MONTH_MAX);
            gtk_spin_button_set_value (GTK_SPIN_BUTTON (button), (gdouble) months);
        }
    }
    else
    {
        widget = g_object_get_data (G_OBJECT (account_page), "bet_config_account_widget");
        if (widget && G_IS_OBJECT (widget))
        {
            g_signal_handlers_block_by_func (G_OBJECT (widget),
                                             G_CALLBACK (utils_widget_duration_button_released),
                                             button);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), TRUE);
        }
        if (button)
        {
            gtk_spin_button_set_range (GTK_SPIN_BUTTON (button), 1.0, PREV_MONTH_MAX / 12.0);
            gtk_spin_button_set_value (GTK_SPIN_BUTTON (button), (gdouble) months / 12.0);
        }
    }

    if (widget && G_IS_OBJECT (widget))
        g_signal_handlers_unblock_by_func (G_OBJECT (widget),
                                           G_CALLBACK (utils_widget_duration_button_released),
                                           button);
    if (button && G_IS_OBJECT (button))
        g_signal_handlers_unblock_by_func (G_OBJECT (button),
                                           G_CALLBACK (utils_widget_duration_number_changed),
                                           ptr);

}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_bet_account_initialise_loan_data (gint account_number,
														 PrefsPageBetAccount *page)
{
	GtkWidget *label;
	GtkWidget *widget;
	GSList *tmp_list;
	gint nbre_pages = 0;
	PrefsPageBetAccountPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_bet_account_get_instance_private (page);

	/* on initialise le notebook si nécessaire */
	nbre_pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook_credit_data));
	if (nbre_pages > 0)
	{
		do
		{
			nbre_pages--;
			gtk_notebook_remove_page (GTK_NOTEBOOK (priv->notebook_credit_data), nbre_pages);
		}
		while (nbre_pages > 0);
	}

	/* Si premier crédit on crée le premier onglet du prêt initial */
	tmp_list = bet_data_loan_get_loan_list_by_account (account_number);
	if (tmp_list == NULL)
	{
		LoanStruct *s_loan;

		s_loan = bet_data_loan_struct_loan_init ();
		s_loan->account_number = account_number;
		s_loan->version_number = -1;
		widget = GTK_WIDGET (widget_loan_new (s_loan));
		label = gtk_label_new (_("Initial loan"));
		utils_widget_set_padding (label, MARGIN_BOX, 0);
		gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_credit_data), widget, label);

		return;
	}

	/* Si le crédit existe on crée le ou les onglets du prêt initial et des renégotiations */
	while (tmp_list)
	{
		gchar *tmp_str;
		gint index = 0;
		LoanStruct *s_loan;

		s_loan = (LoanStruct *) tmp_list->data;
		index = s_loan->version_number;
		widget = GTK_WIDGET (widget_loan_new (s_loan));
		if (!index)
			label = gtk_label_new (_("Initial loan"));
		else
		{
			tmp_str = g_strdup_printf (_("Renegotiation N° %d"), index);
			label = gtk_label_new (tmp_str);
			g_free (tmp_str);
		}

		utils_widget_set_padding (label, MARGIN_BOX, 0);
		gtk_notebook_insert_page (GTK_NOTEBOOK (priv->notebook_credit_data), widget, label, index);

		tmp_list = tmp_list->next;
	}
}

static void prefs_page_bet_account_initialise_select_hist_data (gint account_number,
                                                                      GtkWidget *notebook)
{
    GtkWidget *widget;
    GtkWidget *button = NULL;
    gint param;

    param = gsb_data_account_get_bet_hist_data (account_number);

    if (param == 1)
        button = g_object_get_data (G_OBJECT (notebook), "bet_config_hist_button_2");
    else
        button = g_object_get_data (G_OBJECT (notebook), "bet_config_hist_button_1");

    if (button)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

    param = gsb_data_account_get_bet_hist_fyear (account_number);
    widget = g_object_get_data (G_OBJECT (notebook), "bet_config_hist_fyear_combo");
    bet_hist_set_fyear_from_combobox (widget, param);
}

/**
 * montre ou cache les paramètres en fonction du type d'onglet
 *
 * \param	account_number
 * \param	show
 * \param	page	-> widgets
 *
 * \return
 **/
static void prefs_page_bet_account_show_hide_parameters (gint account_number,
														 gboolean show,
														 PrefsPageBetAccount *page)
{
	PrefsPageBetAccountPrivate *priv;

	priv = prefs_page_bet_account_get_instance_private (page);

    if (show)
    {
		BetTypeOnglets bet_show_onglets;

        bet_show_onglets = gsb_data_account_get_bet_show_onglets (account_number);
        switch (bet_show_onglets)
        {
			case BET_ONGLETS_PREV:
				gtk_widget_hide (priv->hbox_bet_credit_card);
				gtk_widget_show (priv->hbox_forecast_data);
				gtk_widget_show (priv->hbox_hist_data);
				gtk_widget_hide (priv->vbox_liabilities_account);
				break;
			case BET_ONGLETS_HIST:
				gtk_widget_hide (priv->hbox_bet_credit_card);
				gtk_widget_hide (priv->hbox_forecast_data);
				gtk_widget_show (priv->hbox_hist_data);
				gtk_widget_hide (priv->vbox_liabilities_account);
				break;
			case BET_ONGLETS_CAP:
				gtk_widget_show (priv->hbox_bet_credit_card);
				gtk_widget_hide (priv->hbox_forecast_data);
				gtk_widget_hide (priv->hbox_hist_data);
				gtk_widget_show (priv->vbox_liabilities_account);
				break;
			case BET_ONGLETS_SANS:
			case BET_ONGLETS_ASSET:
			default:
				gtk_widget_hide (priv->hbox_bet_credit_card);
				gtk_widget_hide (priv->hbox_forecast_data);
				gtk_widget_hide (priv->hbox_hist_data);
				gtk_widget_hide (priv->vbox_liabilities_account);
				break;
        }
    }
    else
    {
        gtk_widget_hide (priv->hbox_bet_credit_card);
        gtk_widget_hide (priv->hbox_forecast_data);
        gtk_widget_hide (priv->hbox_hist_data);
        gtk_widget_hide (priv->vbox_liabilities_account);
    }
}

/**
 * callback checkbutton select_bank_card
 *
 * \param toggle button
 * \param page
 *
 * \return
 **/
static void prefs_page_bet_account_select_bank_card_toggle (GtkToggleButton *button,
															PrefsPageBetAccount *page)
{
    gint account_number;
	PrefsPageBetAccountPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_bet_account_get_instance_private (page);

    account_number = gsb_account_get_combo_account_number (priv->combo_bet_account);

	if (gtk_toggle_button_get_active (button))
	{
		gsb_data_account_set_bet_credit_card (account_number, 1);
	}
	else
	{
		gchar *msg;

		gsb_data_account_set_bet_credit_card (account_number, 0);
		msg = g_strdup (_("Warning: You are about to delete this deferred debit card: \"%s\"\n"
						  "Are you sure?"));;
		if (dialogue_yes_no (msg, _("Confirmation of deletion of a deferred debit card"), GTK_RESPONSE_CANCEL))
		{
			bet_data_transfert_remove_line_from_card (account_number);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), FALSE);
		}
		else
		{
			gsb_data_account_set_bet_credit_card (account_number, 1);
			g_signal_handlers_block_by_func (G_OBJECT (button),
											 G_CALLBACK (prefs_page_bet_account_select_bank_card_toggle),
											 page);
			gtk_toggle_button_set_active (button, TRUE);
			g_signal_handlers_unblock_by_func (G_OBJECT (button),
											   G_CALLBACK (prefs_page_bet_account_select_bank_card_toggle),
											   page);
		}
		g_free (msg);
	}

	gsb_data_account_set_bet_show_onglets (account_number);
    prefs_page_bet_account_changed (priv->combo_bet_account, page);

    if (gsb_gui_navigation_get_current_account () == account_number)
        bet_data_select_bet_pages (account_number);
}

/**
 * callback checkbutton_bet_hist_use_data_in_account
 *
 * \param toggle button
 * \param page
 *
 * \return
 **/
static void prefs_page_bet_account_use_data_in_account_toggle (GtkToggleButton *button,
															   PrefsPageBetAccount *page)
{
	GHashTable *transfert_list;
	GHashTableIter iter;
	gpointer key, value;
	gint account_number;
	gint number = 0;
	PrefsPageBetAccountPrivate *priv;
	TransfertData *std;

	priv = prefs_page_bet_account_get_instance_private (page);
	devel_debug_int (gtk_toggle_button_get_active (button));

	account_number = gsb_account_get_combo_account_number (priv->combo_bet_account);
	transfert_list = bet_data_transfert_get_list ();
	g_hash_table_iter_init (&iter, transfert_list);

	if (gtk_toggle_button_get_active (button))
	{
		while (g_hash_table_iter_next (&iter, &key, &value))
		{
			std = (TransfertData *) value;

			if (std->main_account_number == account_number)
			{
				number++;
				gsb_data_account_set_bet_hist_use_data_in_account (std->card_account_number, 1);
				gtk_widget_show (priv->vbox_hist_main_account_use_data);
			}
		}
		gsb_data_account_set_bet_hist_use_data_in_account (account_number, number);
		prefs_page_bet_account_set_label_bet_hist_main_account_use_data (number, priv);
	}
	else
	{
		gsb_data_account_set_bet_hist_use_data_in_account (account_number, 0);
		while (g_hash_table_iter_next (&iter, &key, &value))
		{
			std = (TransfertData *) value;

			if (std->main_account_number == account_number)
			{
				gsb_data_account_set_bet_hist_use_data_in_account (std->card_account_number, 0);
				gtk_widget_hide (priv->vbox_hist_main_account_use_data);
			}
		}
	}

	gsb_data_account_set_bet_show_onglets (account_number);
	prefs_page_bet_account_changed (priv->combo_bet_account, page);

    gsb_data_account_set_bet_maj (account_number, BET_MAJ_ALL);
    bet_data_update_bet_module (account_number, -1);

	if (gsb_gui_navigation_get_current_account () == account_number)
		bet_data_select_bet_pages (account_number);
}

/**
 * callback called when changing the account from the combo
 * re-fill the page with the account data
 *
 * \param button
 * \param
 *
 * \return FALSE
 **/
static gboolean prefs_page_bet_account_changed (GtkWidget *combo,
												PrefsPageBetAccount *page)
{
    GtkWidget *account_page;
    gint account_number;
    gint bet_use_budget;
    gint active;
	gint number = 0;
	KindAccount kind;
	GrisbiWinEtat *w_etat;
	PrefsPageBetAccountPrivate *priv;

	priv = prefs_page_bet_account_get_instance_private (page);
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

    account_number = gsb_account_get_combo_account_number (combo);
	account_page = grisbi_win_get_account_page ();

    /* on bloque l'appel aux fonctions de callback */
    g_signal_handlers_block_by_func (G_OBJECT (combo),
									 G_CALLBACK (prefs_page_bet_account_changed),
									 page);
    g_signal_handlers_block_by_func (G_OBJECT (priv->checkbutton_bet_credit_card),
									 G_CALLBACK (prefs_page_bet_account_select_bank_card_toggle),
									 page);
    g_signal_handlers_block_by_func (G_OBJECT (priv->checkbutton_hist_main_account_use_data),
									 G_CALLBACK (prefs_page_bet_account_use_data_in_account_toggle),
									 page);

	bet_use_budget = gsb_data_account_get_bet_use_budget (account_number);
    switch (bet_use_budget)		/* -1 = pas de module possible 0 = non utilisé 1 = utilisé */
    {
        case -1:
            gtk_widget_set_sensitive (priv->checkbutton_use_bet_module, FALSE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), FALSE);
            prefs_page_bet_account_show_hide_parameters (account_number, FALSE, page);
            goto retour;
            break;
        case 0:
            gtk_widget_set_sensitive (priv->checkbutton_use_bet_module, TRUE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), FALSE);
            prefs_page_bet_account_show_hide_parameters (account_number, FALSE, page);
            goto retour;
            break;
        default:
            gtk_widget_set_sensitive (priv->checkbutton_use_bet_module, TRUE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), TRUE);
            prefs_page_bet_account_show_hide_parameters (account_number, TRUE, page);
            break;
    }

	/* on cache les deux vbox pour l'aggregation des données historiques*/
	gtk_widget_hide (priv->vbox_hist_card_account_use_data);
	gtk_widget_hide (priv->vbox_hist_main_account_use_data);

	kind = gsb_data_account_get_kind (account_number);
    switch (kind)
    {
		case GSB_TYPE_BANK:
            gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_bet_account), 0);
            prefs_page_bet_account_initialise_duration_widget (account_number, account_page);
            prefs_page_bet_account_initialise_select_hist_data (account_number, account_page);
			gtk_widget_show (priv->vbox_hist_main_account_use_data);

			/* init checkbutton_hist_main_account_use_data */
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_hist_main_account_use_data),
										  gsb_data_account_get_bet_hist_use_data_in_account (account_number));
			break;
		case GSB_TYPE_CASH:
			if (w_etat->bet_cash_account_option == 1)
			{
				gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_bet_account), 0);
				prefs_page_bet_account_initialise_duration_widget (account_number, account_page);
				prefs_page_bet_account_initialise_select_hist_data (account_number, account_page);
			}
			else
			{
				gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_bet_account), 0);
				prefs_page_bet_account_initialise_select_hist_data (account_number, account_page);
			}
			break;
		case GSB_TYPE_LIABILITIES:
			gtk_widget_show (priv->hbox_bet_credit_card);
			active = gsb_data_account_get_bet_credit_card (account_number);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_bet_credit_card), active);
			if (active)
			{
				/* affiche un label si données partagées dans le compte principal */
				number = gsb_data_account_get_bet_hist_use_data_in_account (account_number);
				if (number)
				{
					gtk_widget_show (priv->vbox_hist_card_account_use_data);
				}

				gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_bet_account), 0);
				if (w_etat->bet_cash_account_option == 1)
				{
					prefs_page_bet_account_initialise_duration_widget (account_number, account_page);
				}
				prefs_page_bet_account_initialise_select_hist_data (account_number, account_page);
			}
			else
			{
				gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_bet_account), 1);
				prefs_page_bet_account_initialise_loan_data (account_number, page);
			}
			break;
		case GSB_TYPE_BALANCE:
		case GSB_TYPE_ASSET:
        default:
            gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_bet_account), 0);
            break;
    }

retour:
    /* on débloque les callbacks */
	g_signal_handlers_unblock_by_func (G_OBJECT (combo),
									   G_CALLBACK (prefs_page_bet_account_changed),
									   page);
	g_signal_handlers_unblock_by_func (G_OBJECT (priv->checkbutton_bet_credit_card),
									   G_CALLBACK (prefs_page_bet_account_select_bank_card_toggle),
									   page);
	g_signal_handlers_unblock_by_func (G_OBJECT (priv->checkbutton_hist_main_account_use_data),
									   G_CALLBACK (prefs_page_bet_account_use_data_in_account_toggle),
									   page);
    /* return */
    return FALSE;
}

/**
 * callback called when changing the account from the button
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_bet_account_use_budget_toggle (GtkToggleButton *button,
													  PrefsPageBetAccount *page)
{
	gint account_number;
	PrefsPageBetAccountPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_bet_account_get_instance_private (page);

    account_number = gsb_account_get_combo_account_number (priv->combo_bet_account);

    if (gtk_toggle_button_get_active (button))
    {
        gsb_data_account_set_bet_use_budget (account_number, 1);
        gsb_data_account_set_bet_show_onglets (account_number);
        prefs_page_bet_account_changed (priv->combo_bet_account, page);
		gtk_widget_show (priv->hbox_bet_credit_card);
		gtk_widget_show (priv->notebook_bet_account);
        gsb_data_account_set_bet_maj (account_number, BET_MAJ_ALL);
    }
    else
    {
        bet_data_remove_all_bet_data (account_number);
        gsb_data_account_set_bet_use_budget (account_number, 0);
		gsb_data_account_set_bet_show_onglets (account_number);
        prefs_page_bet_account_show_hide_parameters (account_number, FALSE, page);
		gtk_widget_hide (priv->hbox_bet_credit_card);
		gtk_widget_hide (priv->notebook_bet_account);
    }

    if (gsb_gui_navigation_get_current_account () == account_number)
        bet_data_select_bet_pages (account_number);
}

/**
 * Création de la page de gestion des bet_account
 *
 * \param page
 *
 * \return
 **/
static void prefs_page_bet_account_setup_account_page (PrefsPageBetAccount *page)
{
    GtkWidget *account_page;
	GtkWidget *combo;
	GtkWidget *head_page;
	GtkWidget *label;
	GtkWidget *widget;
    gint account_number;
	gint bet_use_budget;
	gboolean is_loading;
	PrefsPageBetAccountPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_bet_account_get_instance_private (page);
	is_loading = grisbi_win_file_is_loading ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Accounts data"), "gsb-balance_estimate-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_bet_account), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_bet_account), head_page, 0);

	if (is_loading == FALSE)
	{
		combo = utils_prefs_create_combo_list_indisponible ();
		gtk_box_pack_start (GTK_BOX (priv->hbox_bet_select_account), combo, FALSE, FALSE, 0);
		gtk_box_reorder_child (GTK_BOX (priv->hbox_bet_select_account), combo, 1);
		gtk_widget_set_sensitive (priv->vbox_bet_account, FALSE);
		return;
	}

	/* set the choice of account */
	account_page = grisbi_win_get_account_page ();
	combo = gsb_account_create_combo_list (NULL, NULL, FALSE);
    g_object_set_data (G_OBJECT (account_page), "account_combo", combo);
	if ((account_number = gsb_gui_navigation_get_current_account ()) == -1)
	{
		gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
		account_number = gsb_account_get_combo_account_number (combo);
	}
	else
		gsb_account_set_combo_account_number (combo, account_number);

	gtk_box_pack_start (GTK_BOX (priv->hbox_bet_select_account), combo, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->hbox_bet_select_account), combo, 1);
	priv->combo_bet_account = combo;

    /* sélectionne un compte carte bancaire à débit différé */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_bet_credit_card), FALSE);

	/* set notebbok */
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (priv->notebook_bet_account), TRUE);
	gtk_box_pack_start (GTK_BOX (priv->vbox_bet_account), priv->notebook_bet_account, FALSE, FALSE, 0);

    /* Data for the accounts of type GSB_TYPE_BANK, GSB_TYPE_CASH */
    gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_bet_account), priv->vbox_bank_cash_account, NULL);

	/* set the data for the checkbutton "Option for cash accounts" */
	g_object_set_data (G_OBJECT (account_page), "bet_credit_card_hbox", priv->hbox_bet_credit_card);
	g_object_set_data (G_OBJECT (account_page), "Data_for_forecast", priv->hbox_forecast_data);
	g_object_set_data (G_OBJECT (account_page), "Data_for_hist", priv->hbox_hist_data);
	g_object_set_data (G_OBJECT (account_page), "Data_for_credit", priv->vbox_liabilities_account);

    /* Data for the forecast */
	/* Calculation of duration */
    widget = utils_widget_get_duration_widget (SPP_ORIGIN_CONFIG);
    gtk_box_pack_start (GTK_BOX (priv->vbox_forecast_data), widget, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_forecast_data), widget, 0);

    /* Select the labels of the list */
	prefs_page_bet_account_init_select_labels_widget (priv);

	/* Sources of historical data */
	widget = utils_widget_origin_data_new (account_page, SPP_ORIGIN_CONFIG);
	gtk_box_pack_start (GTK_BOX (priv->vbox_hist_data), widget, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_hist_data), widget, 0);

    /* Data for the account of type GSB_TYPE_LIABILITIES */
    gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_bet_account), priv->vbox_liabilities_account, NULL);

	/* Data for the credit */
	gtk_notebook_set_scrollable (GTK_NOTEBOOK (priv->notebook_credit_data), TRUE);

	priv->vbox_loan_data = GTK_WIDGET (widget_loan_new (NULL));
	g_object_set_data (G_OBJECT (account_page), "Data_for_credit", priv->vbox_loan_data);
	label = gtk_label_new (_("Initial loan"));
	utils_widget_set_padding (label, MARGIN_BOX, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_credit_data), priv->vbox_loan_data, label);

	/* init_use_bet_module */
	bet_use_budget = gsb_data_account_get_bet_use_budget (account_number);
	if (bet_use_budget == 1)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), TRUE);
		gtk_widget_show (priv->hbox_bet_credit_card);
		gtk_widget_show (priv->notebook_bet_account);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), FALSE);
		gtk_widget_hide (priv->hbox_bet_credit_card);
		gtk_widget_hide (priv->notebook_bet_account);
	}

	/* mettre à jour les données du compte */
    prefs_page_bet_account_changed (combo, page);

	/* set signals */
	/* Connect signal account combo */
    g_signal_connect (G_OBJECT (combo),
					  "changed",
					  G_CALLBACK (prefs_page_bet_account_changed),
					  page);

	/* Connect signal checkbutton_use_bet_module */
    g_signal_connect (G_OBJECT (priv->checkbutton_use_bet_module),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_use_budget_toggle),
					  page);

    /* set the signal checkbutton_bet_credit_card */
    g_signal_connect (G_OBJECT (priv->checkbutton_bet_credit_card),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_select_bank_card_toggle),
					  page);

	/* set the signal checkbutton_bet_hist_use_data_in_account */
    g_signal_connect (G_OBJECT (priv->checkbutton_hist_main_account_use_data),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_use_data_in_account_toggle),
					  page);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_bet_account_init (PrefsPageBetAccount *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));

	prefs_page_bet_account_setup_account_page (page);
}

static void prefs_page_bet_account_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_bet_account_parent_class)->dispose (object);
}

static void prefs_page_bet_account_class_init (PrefsPageBetAccountClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_bet_account_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_bet_account.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_bet_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, hbox_bet_select_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, notebook_bet_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, checkbutton_use_bet_module);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, hbox_bet_credit_card);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, checkbutton_bet_credit_card);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_bank_cash_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, hbox_forecast_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_forecast_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, hbox_hist_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_hist_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_liabilities_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_credit_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, notebook_credit_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_loan_data);

	/* set origin of forecast data */
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  radiobutton_source_transaction0);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  radiobutton_source_transaction1);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  radiobutton_source_transaction2);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  radiobutton_source_transaction3);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  radiobutton_source_scheduled0);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  radiobutton_source_scheduled1);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  radiobutton_source_scheduled2);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  radiobutton_source_scheduled3);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass)
												  ,PrefsPageBetAccount,
												  radiobutton_source_futur0);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  radiobutton_source_futur1);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  radiobutton_source_futur2);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass)
												  ,PrefsPageBetAccount,
												  radiobutton_source_futur3);

	/* ajout pour gerer l'agregation des données dans le compte principal */
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  vbox_hist_card_account_use_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  label_hist_card_account_use_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  vbox_hist_main_account_use_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  checkbutton_hist_main_account_use_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPageBetAccount,
												  label_hist_main_account_use_data);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
PrefsPageBetAccount *prefs_page_bet_account_new (GrisbiPrefs *win)
{
  return g_object_new (PREFS_PAGE_BET_ACCOUNT_TYPE, NULL);
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

