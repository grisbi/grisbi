/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2018 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "prefs_page_bet_account.h"
#include "bet_config.h"
#include "bet_data.h"
#include "bet_data_finance.h"
#include "gsb_account.h"
#include "gsb_calendar_entry.h"
#include "gsb_file.h"
#include "navigation.h"
//~ #include "prefs_widget_loan.h"
#include "structures.h"
#include "utils.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

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
	GtkWidget *			eventbox_use_bet_module;
	GtkWidget *			vbox_bet_selected_account;
	GtkWidget *			hbox_bet_credit_card;
    GtkWidget *			checkbutton_bet_credit_card;
	GtkWidget *			eventbox_bet_credit_card;
	GtkWidget *			vbox_bank_cash_account;
	GtkWidget *			hbox_forecast_data;
	GtkWidget *			vbox_forecast_data;
	GtkWidget *			vbox_historical_data;
	GtkWidget *			vbox_liabilities_account;
	GtkWidget *			vbox_credit_data;
	GtkWidget *			notebook_credit_data;
	GtkWidget *			vbox_loan_data;
	GtkWidget *			grid_loan_data;
	GtkWidget *			spinbutton_rate_interest;

    //~ GtkWidget *			checkbutton_;
	//~ GtkWidget *			eventbox_;
    //~ GtkWidget *         spinbutton_n;
    //~ GtkWidget *         filechooserbutton_;

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
//~ static void prefs_page_bet_account_initialise_loan_data (gint account_number,
														 //~ PrefsPageBetAccount *page)
//~ {
	//~ GtkWidget *label;
	//~ GtkWidget *widget;
	//~ GSList *list;
	//~ gint nbre_pages = 0;
	//~ PrefsPageBetAccountPrivate *priv;

	//~ devel_debug (NULL);
	//~ priv = prefs_page_bet_account_get_instance_private (page);

	//~ /* on initialise le notebook si nécessaire */
	//~ nbre_pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (priv->notebook_credit_data));
	//~ if (nbre_pages > 0)
	//~ {
		//~ do
		//~ {
			//~ nbre_pages--;
			//~ gtk_notebook_remove_page (GTK_NOTEBOOK (priv->notebook_credit_data), nbre_pages);
		//~ }
		//~ while (nbre_pages > 0);
	//~ }

	//~ /* on crée le premier onglet pour la capital initial */
	//~ list = bet_data_loan_get_loan_list_by_account (account_number);
	//~ if (list == NULL)
	//~ {
		//~ widget = GTK_WIDGET (prefs_widget_loan_new (NULL));
		//~ label = gtk_label_new (_("Initial loan"));
		//~ utils_widget_set_padding (label, MARGIN_BOX, 0);
		//~ gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_credit_data), widget, label);

		//~ return;
	//~ }

	//~ while (list)
	//~ {
		//~ LoanStruct *s_loan;

		//~ s_loan = (LoanStruct *) list->data;
		//~ widget = GTK_WIDGET (prefs_widget_loan_new (s_loan));
		//~ label = gtk_label_new (_("Initial loan"));
		//~ utils_widget_set_padding (label, MARGIN_BOX, 0);
		//~ gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_credit_data), widget, label);


		//~ list = list->next;
	//~ }
//~ }

/**
 * positionne l'option credit_card
 *
 * \param account_number
 *
 * \return
 * */
static void prefs_page_bet_account_set_bet_credit_card (GtkWidget *button,
														gint account_number)
{
    gint active;

	active = gsb_data_account_get_bet_credit_card (account_number);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), active);
}

static void prefs_page_bet_account_sensitive_account_parameters (gint account_number,
																 gboolean sensitive,
																 PrefsPageBetAccount *page)
{
	PrefsPageBetAccountPrivate *priv;

	priv = prefs_page_bet_account_get_instance_private (page);

    if ( sensitive )
    {
		BetTypeOnglets bet_show_onglets;

        bet_show_onglets = gsb_data_account_get_bet_show_onglets ( account_number );

        switch ( bet_show_onglets )
        {
			case BET_ONGLETS_PREV:
				gtk_widget_show (priv->hbox_bet_credit_card);
				gtk_widget_show (priv->hbox_forecast_data);
				gtk_widget_show (priv->vbox_historical_data);
				//~ gtk_widget_hide (priv->vbox_credit_data);
				break;
			case BET_ONGLETS_HIST:
				gtk_widget_show (priv->hbox_bet_credit_card);
				gtk_widget_hide (priv->hbox_forecast_data);
				gtk_widget_show (priv->vbox_historical_data);
				//~ gtk_widget_hide (priv->vbox_credit_data);
				break;
			case BET_ONGLETS_CAP:
				gtk_widget_show (priv->hbox_bet_credit_card);
				gtk_widget_hide (priv->hbox_forecast_data);
				gtk_widget_hide (priv->vbox_historical_data);
				gtk_widget_show (priv->vbox_credit_data);
				break;
			default:
				gtk_widget_hide (priv->hbox_bet_credit_card);
				gtk_widget_hide (priv->hbox_forecast_data);
				gtk_widget_hide (priv->vbox_historical_data);
				gtk_widget_hide (priv->vbox_credit_data);
				break;
        }
    }
    else
    {
        gtk_widget_hide (priv->hbox_bet_credit_card);
        gtk_widget_hide (priv->hbox_forecast_data);
        gtk_widget_hide (priv->vbox_historical_data);
        gtk_widget_hide (priv->vbox_credit_data);
    }
}

/**
 * callback called when changing the account from the combo
 * re-fill the page with the account data
 *
 * \param button
 * \param
 *
 * \return FALSE
 * */
static gboolean prefs_page_bet_account_changed (GtkWidget *combo,
												PrefsPageBetAccount *page)
{
    //~ GtkWidget *fyear_combo;
    GtkWidget *account_page;
    gint account_number;
    gint bet_use_budget;
    BetTypeOnglets bet_show_onglets;
	PrefsPageBetAccountPrivate *priv;

	priv = prefs_page_bet_account_get_instance_private (page);

    account_number = gsb_account_get_combo_account_number (combo);
	devel_debug_int (account_number);
    bet_use_budget = gsb_data_account_get_bet_use_budget (account_number);
    account_page = grisbi_win_get_account_page ();

    //~ /* on bloque l'appel à la fonction bet_config_fyear_clicked */
    //~ fyear_combo = g_object_get_data (G_OBJECT (account_page), "bet_config_hist_fyear_combo");
    //~ g_signal_handlers_block_by_func (G_OBJECT (fyear_combo),
									 //~ G_CALLBACK (bet_config_fyear_clicked),
									 //~ GINT_TO_POINTER (0));

    switch (bet_use_budget)
    {
        case -1:
            gtk_widget_set_sensitive (priv->checkbutton_use_bet_module, FALSE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), FALSE);
            prefs_page_bet_account_sensitive_account_parameters (account_number, FALSE, page);
            return FALSE;
            break;
        case 0:
            gtk_widget_set_sensitive (priv->checkbutton_use_bet_module, TRUE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), FALSE);
            prefs_page_bet_account_sensitive_account_parameters (account_number, FALSE, page);
            return FALSE;
            break;
        default:
            gtk_widget_set_sensitive (priv->checkbutton_use_bet_module, TRUE);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), TRUE);
            prefs_page_bet_account_sensitive_account_parameters (account_number, TRUE, page);
            break;
    }

    bet_show_onglets = gsb_data_account_get_bet_show_onglets (account_number);
    switch (bet_show_onglets)
    {
        case BET_ONGLETS_PREV:
            gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_bet_account), 0);
            prefs_page_bet_account_set_bet_credit_card (priv->checkbutton_bet_credit_card, account_number);
            bet_config_initialise_duration_widget (account_number, account_page);
            bet_config_initialise_select_historical_data (account_number, account_page);
			gtk_widget_set_sensitive (priv->hbox_bet_credit_card, TRUE);
            break;
        case BET_ONGLETS_HIST:
            gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_bet_account), 0);
            prefs_page_bet_account_set_bet_credit_card (priv->checkbutton_bet_credit_card, account_number);
            bet_config_initialise_select_historical_data (account_number, account_page);
			gtk_widget_set_sensitive (priv->hbox_bet_credit_card, TRUE);
            break;
        case BET_ONGLETS_CAP:
            gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_bet_account), 1);
            prefs_page_bet_account_set_bet_credit_card (priv->checkbutton_bet_credit_card, account_number);
            //~ prefs_page_bet_account_initialise_loan_data (account_number, page);
			bet_config_initialise_finance_widget (account_number, account_page);
			gtk_widget_set_sensitive (priv->hbox_bet_credit_card, FALSE);
            break;
        default:
            gtk_notebook_set_current_page (GTK_NOTEBOOK (priv->notebook_bet_account), 0);
            break;
    }

    /* on débloque l'appel à la fonction bet_config_fyear_clicked */
    //~ g_signal_handlers_unblock_by_func (G_OBJECT (fyear_combo),
									   //~ G_CALLBACK (bet_config_fyear_clicked),
									   //~ GINT_TO_POINTER (0));

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

    if ( gtk_toggle_button_get_active ( button ) )
    {
        gsb_data_account_set_bet_use_budget ( account_number, 1 );
        gsb_data_account_set_bet_show_onglets ( account_number );
        prefs_page_bet_account_changed (priv->combo_bet_account, page);

        gsb_data_account_set_bet_maj ( account_number, BET_MAJ_ALL );
    }
    else
    {
        bet_data_remove_all_bet_data ( account_number );
        gsb_data_account_set_bet_use_budget ( account_number, 0 );
        prefs_page_bet_account_sensitive_account_parameters ( account_number, FALSE, page );
    }

    if ( gsb_gui_navigation_get_current_account ( ) == account_number )
        bet_data_select_bet_pages ( account_number );
}

/**
 * Création de la page de gestion des bet_account
 *
 * \param prefs
 *
 * \return
 */
static void prefs_page_bet_account_setup_account_page (PrefsPageBetAccount *page)
{
    GtkWidget *account_page;
	GtkWidget *combo;
	GtkWidget *head_page;
	//~ GtkWidget *label;
	GtkWidget *widget;
    gint account_number;
	gint bet_use_budget;
	PrefsPageBetAccountPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_bet_account_get_instance_private (page);
    account_page = grisbi_win_get_account_page ();

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Accounts data"), "gsb-balance_estimate-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_bet_account), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_bet_account), head_page, 0);

    /* set the choice of account */
	combo = gsb_account_create_combo_list (G_CALLBACK (prefs_page_bet_account_changed), page, FALSE);
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

	/* init_use_bet_module */
	bet_use_budget = gsb_data_account_get_bet_use_budget (account_number);
	if (bet_use_budget == 1)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), TRUE);
		gtk_widget_show (priv->vbox_bet_selected_account);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_use_bet_module), FALSE);
		gtk_widget_hide (priv->vbox_bet_selected_account);
	}

    /* sélectionne un compte carte bancaire à débit différé */
    //~ gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_bet_credit_card), FALSE);

    /* set the signal */
    //~ g_signal_connect (G_OBJECT (priv->checkbutton_bet_credit_card),
					  //~ "toggled",
					  //~ G_CALLBACK (bet_config_select_bank_card_toggle),
					  //~ combo);
    //~ g_signal_connect (priv->eventbox_bet_credit_card,
					  //~ "button-press-event",
					  //~ G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  //~ priv->checkbutton_bet_credit_card);

	/* set notebbok */
	//~ gtk_notebook_set_scrollable (GTK_NOTEBOOK (priv->notebook_bet_account), TRUE);
	//~ gtk_box_pack_start (GTK_BOX (priv->vbox_bet_account), priv->notebook_bet_account, FALSE, FALSE, 0);

    /* Data for the accounts of type GSB_TYPE_BANK, GSB_TYPE_CASH */
    //~ gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_bet_account), priv->vbox_bank_cash_account, NULL);

    /* Data for the forecast */
	/* Calculation of duration */
    //~ widget = bet_config_get_duration_widget ( SPP_ORIGIN_CONFIG );
    //~ gtk_box_pack_start (GTK_BOX (priv->vbox_forecast_data), widget, FALSE, FALSE, 0);

    /* Select the labels of the list */
    //~ bet_config_get_select_labels_widget (priv->vbox_forecast_data);

    /* Sources of historical data */
	//~ bet_config_get_select_historical_data (priv->vbox_historical_data, account_page);

    /* Data for the account of type GSB_TYPE_LIABILITIES */
    //~ gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_bet_account), priv->vbox_liabilities_account, NULL);


	/* Data for the credit */
	//~ gtk_notebook_set_scrollable (GTK_NOTEBOOK (priv->notebook_credit_data), TRUE);

	//~ priv->vbox_loan_data = GTK_WIDGET (prefs_widget_loan_new ());
	//~ label = gtk_label_new (_("Initial loan"));
	//~ utils_widget_set_padding (label, MARGIN_BOX, 0);
	//~ gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_credit_data), priv->vbox_loan_data, label);

	/* onglet new */
	//~ label = gtk_label_new ("+");
	//~ widget = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	//~ gtk_widget_show (widget);
	//~ gtk_notebook_append_page (GTK_NOTEBOOK (priv->notebook_credit_data), widget, label);

	/* Date of first Repayment */
	//~ widget = gsb_calendar_entry_new ( FALSE );
    //~ g_object_set_data ( G_OBJECT ( parent ), "bet_config_start_date", widget );
	//~ gtk_grid_attach (GTK_GRID (priv->grid_loan_data), widget, 1, 5, 1, 1);

	/* Annuel rate interest */
	//~ gtk_spin_button_set_digits (GTK_SPIN_BUTTON (priv->spinbutton_rate_interest), BET_TAUX_DIGITS);

	/* set signals */
	/* Connect signal checkbutton_use_bet_module */
    g_signal_connect (G_OBJECT (priv->checkbutton_use_bet_module),
					  "toggled",
					  G_CALLBACK (prefs_page_bet_account_use_budget_toggle),
					  page);
    g_signal_connect (priv->eventbox_use_bet_module,
					  "button-press-event",
					  G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  priv->checkbutton_use_bet_module);

	/* mettre à jour les données du compte */
    prefs_page_bet_account_changed (combo, page);

	/* set the variables for account */
    //~ gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_),
								  //~ conf.);
	/* set spinbutton value */
	//~ gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_),
							   //~ conf.);


    /* Connect signal */
    //~ g_signal_connect (priv->eventbox_,
					  //~ "button-press-event",
					  //~ G_CALLBACK (utils_prefs_page_eventbox_clicked),
					  //~ priv->checkbutton_);

    //~ g_signal_connect (priv->checkbutton_,
					  //~ "toggled",
					  //~ G_CALLBACK (utils_prefs_page_checkbutton_changed),
					  //~ &conf.);

    /* callback for spinbutton_ */
    //~ g_object_set_data (G_OBJECT (priv->spinbutton_),
                       //~ "button", priv->checkbutton_);
	//~ g_object_set_data (G_OBJECT (priv->checkbutton_),
                       //~ "spinbutton", priv->spinbutton_);

    //~ g_signal_connect (priv->spinbutton_,
					  //~ "value-changed",
					  //~ G_CALLBACK (utils_prefs_spinbutton_changed),
					  //~ &conf.);

    /* connect the signal for filechooserbutton_backup */
    //~ g_signal_connect (G_OBJECT (priv->filechooserbutton_backup),
                      //~ "selection-changed",
                      //~ G_CALLBACK (utils_prefs_page_dir_chosen),
                      //~ "backup_path");

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
												 "/org/gtk/grisbi/ui/prefs_page_bet_account.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_bet_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, hbox_bet_select_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_bet_selected_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, notebook_bet_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, checkbutton_use_bet_module);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, eventbox_use_bet_module);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, hbox_bet_credit_card);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, checkbutton_bet_credit_card);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, eventbox_bet_credit_card);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_bank_cash_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, hbox_forecast_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_forecast_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_historical_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_liabilities_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_credit_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, notebook_credit_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, vbox_loan_data);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, grid_loan_data);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, spinbutton_rate_interest);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, checkbutton_);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, eventbox);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, spinbutton_);
	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), PrefsPageBetAccount, filechooserbutton_);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
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

