/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2021 Pierre Biava (grisbi@pierre.biava.name)                    */
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
/*     along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_loan.h"
#include "bet_finance_ui.h"
#include "grisbi_app.h"
#include "gsb_data_currency.h"
#include "gsb_calendar_entry.h"
#include "gsb_combo_box.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_form.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_scheduled.h"
#include "gsb_dirs.h"
#include "gsb_form_widget.h"
#include "gsb_payment_method.h"
#include "gtk_combofix.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_prefs.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/


typedef struct _WidgetLoanPrivate	WidgetLoanPrivate;
typedef struct _DialogScheduled   	DialogScheduled;

struct _WidgetLoanPrivate
{
	GtkWidget *			vbox_loan_data;
	GtkWidget *			grid_loan_data;

	GtkAdjustment *		adjustment_rate_interest;

	GtkWidget *			button_amortization_table;
	GtkWidget *			button_init_scheduled;

	GtkWidget *			checkbutton_amount_first_is_different;
	GtkWidget *			checkbutton_init_sch_with_loan;
	GtkWidget *			checkbutton_invers_cols_cap_ech;

	GtkWidget *			combo_account_list;

	GtkWidget *			entry_amount_first_capital;
	GtkWidget *			entry_amount_first_fees;
	GtkWidget *			entry_amount_first_interests;
	GtkWidget *			entry_fees_per_month;
	GtkWidget *			entry_first_date;
	GtkWidget *			entry_fixed_due_amount;
	GtkWidget *			entry_loan_capital;

	GtkWidget *			label_fees_per_month_devise;
	GtkWidget *			label_first_capital_devise;
	GtkWidget *			label_first_fees_devise;
	GtkWidget *			label_first_interests_devise;
	GtkWidget *			label_fixed_amount;
	GtkWidget *			label_fixed_amount_devise;
	GtkWidget *			label_loan_capital_devise;

	GtkWidget *			radiobutton_type_taux_1;
	GtkWidget *			radiobutton_type_taux_2;
	GtkWidget *			radiobutton_type_taux_3;

	GtkWidget *			spinbutton_loan_duration;
	GtkWidget *			spinbutton_percentage_fees;
	GtkWidget *			spinbutton_rate_interest;

	GtkWidget *			tree_view_amortization_table;
	GtkWidget *			vbox_fees_per_month;

	gchar *				code_devise;

	LoanStruct *		s_loan;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetLoan, widget_loan, GTK_TYPE_BOX)

struct _DialogScheduled
{
	GtkWidget *label;
	GtkWidget *paddingbox;
	GtkWidget *hbox;
	GtkWidget *AccountListCombo;
	GtkWidget *ACategCombo;
	GtkWidget *AFreqencyCombo;
	GtkWidget *APayeeCombo;
	GtkWidget *APaymentCombo;
	GtkWidget *FBudgetCombo;
	GtkWidget *FCategCombo;
	GtkWidget *SBudgetCombo;
	GtkWidget *SCategCombo;
	GtkWidget *TBudgetCombo;
	GtkWidget *TCategCombo;
	GtkWidget *separator;
	GtkWidget *icon;
};

/*START_STATIC*/
static void widget_loan_button_init_scheduled_clicked (GtkButton *button,
													   WidgetLoan *w_loan);

/* memorisation des date */
static gchar *old_entry = NULL;
static gdouble old_number = 0.0;
/* pixbuf pour les gtk_entry */
static GdkPixbuf *pixbuf_OK;
static GdkPixbuf *pixbuf_NOK;
/*END_STATIC*/

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
static void widget_loan_adapt_display_from_type_taux (gint type_taux,
													  WidgetLoanPrivate *priv)
{
	devel_debug (NULL);
	if (type_taux == 2)
	{
		gtk_label_set_text (GTK_LABEL (priv->label_fees_per_month_devise), "%");
		gtk_widget_hide (priv->entry_fees_per_month);
		gtk_widget_show (priv->entry_fixed_due_amount);
		gtk_widget_show (priv->label_fixed_amount);
		gtk_widget_show (priv->spinbutton_percentage_fees);
		gtk_widget_set_sensitive (priv->entry_amount_first_fees, TRUE);
		gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_fees), "");
		gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_fees),
										GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
	}
	else
	{
		LoanStruct *s_loan;

		s_loan = priv->s_loan;
		gtk_label_set_text (GTK_LABEL (priv->label_fees_per_month_devise), priv->code_devise);
		gtk_widget_hide (priv->entry_fixed_due_amount);
		gtk_widget_hide (priv->label_fixed_amount);
		gtk_widget_hide (priv->spinbutton_percentage_fees);
		gtk_widget_show (priv->entry_fees_per_month);
		gtk_widget_set_sensitive (priv->entry_amount_first_fees, FALSE);
		gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_fees), "");
		if (s_loan->first_is_different)
		{
			gchar *tmp_str;
			GsbReal real;

			real = gsb_real_double_to_real (s_loan->amount_fees);
			if (real.mantissa > 0)
			{
				tmp_str = utils_real_get_string_with_currency (gsb_real_double_to_real (s_loan->amount_fees),
															   s_loan->currency_number,
															   TRUE);
				gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_fees), tmp_str);
				g_free  (tmp_str);
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_fees),
												GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
			}
			else
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_fees),
												GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
		}
		else
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_fees),
											GTK_ENTRY_ICON_PRIMARY, NULL);
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
static void widget_loan_checkbutton_invers_cols_cap_ech (GtkToggleButton *togglebutton,
														 WidgetLoan *w_loan)
{
	WidgetLoanPrivate *priv;
	LoanStruct *s_loan;

	devel_debug (NULL);

	priv = widget_loan_get_instance_private (w_loan);
	s_loan = priv->s_loan;
	s_loan->invers_cols_cap_ech = gtk_toggle_button_get_active (togglebutton);
	if (gsb_gui_navigation_get_current_account () == s_loan->account_number)
	{
		GtkWidget *account_page;

		account_page = grisbi_win_get_account_page ();
		if (gtk_notebook_get_current_page (GTK_NOTEBOOK (account_page)) == GSB_FINANCE_PAGE)
			bet_finance_update_amortization_tab_with_data (s_loan->account_number, account_page, s_loan);
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static DialogScheduled *widget_loan_scheduled_dialog_struct_init (void)
{
	DialogScheduled	*s_sch_dialog;

	s_sch_dialog = g_malloc0 ( sizeof (DialogScheduled));

	return s_sch_dialog;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_loan_scheduled_dialog_struct_free (DialogScheduled	*s_sch_dialog)
{
	g_free (s_sch_dialog);
}

/**
 * Valide les données obligatoires
 *
 * \param
 *
 * \return TRUE if valide else FALSE
 **/
static gboolean widget_loan_scheduled_dialog_validate (DialogScheduled *s_sch_dialog)
{
	gboolean valide_payee = FALSE;
	gboolean valide_second_line = FALSE;
	gboolean valide_third_line = FALSE;

	if (!gsb_form_widget_check_empty (s_sch_dialog->APayeeCombo))
	{
		valide_payee = TRUE;
	}
	if (!gsb_form_widget_check_empty (s_sch_dialog->SCategCombo)
		|| !gsb_form_widget_check_empty (s_sch_dialog->SBudgetCombo))
	{
		valide_second_line = TRUE;
	}
	if (!gsb_form_widget_check_empty (s_sch_dialog->TCategCombo)
		|| !gsb_form_widget_check_empty (s_sch_dialog->TBudgetCombo))
	{
		valide_third_line = TRUE;
	}

	if (valide_payee && valide_second_line && valide_third_line)
		return TRUE;
	else
		return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gint widget_loan_combo_frequency_get_frequency (gint index)
{
	switch (index)
	{
		case 1:
			index = 4;
			break;
		case 2:
			index = 5;
			break;
		default:
			index = 2;
	}

	return index;
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
static gboolean widget_loan_combofix_enter_focus (GtkWidget *entry,
														GdkEventFocus *ev,
														gpointer user_data)
{
    /* the entry is a real entry */
	if (gsb_form_widget_check_empty (entry))
	{
		gtk_entry_set_text (GTK_ENTRY (entry), "");
		gsb_form_widget_set_empty (entry, FALSE);
	}

	return FALSE;
}

/**
 * called when an entry lose the focus
 *
 * \param entry
 * \param ev
 * \param combo
 *
 * \return FALSE
 **/
static gboolean widget_loan_combofix_lose_focus (GtkWidget *entry,
												 GdkEventFocus *ev,
												 GtkWidget *combo)
{
    const gchar *string;
    gint element_number;
	gpointer ptr_origin;

	/* still not found, if change the content of the form, something come in entry
     * which is nothing, so protect here */
    if (!GTK_IS_WIDGET (entry) || !GTK_IS_ENTRY (entry))
        return FALSE;

    /* remove the selection */
    gtk_editable_select_region (GTK_EDITABLE (entry), 0, 0);

	/* get element widget */
	ptr_origin = g_object_get_data (G_OBJECT (combo), "element");
    element_number = GPOINTER_TO_INT (ptr_origin);

    /* sometimes the combofix popup stays showed, so remove here */
    if ( element_number == TRANSACTION_FORM_PARTY
		|| element_number == TRANSACTION_FORM_CATEGORY
		|| element_number == TRANSACTION_FORM_BUDGET)
    {
        gtk_combofix_hide_popup (GTK_COMBOFIX (combo));
    }

    /* string will be filled only if the field is empty */
    string = NULL;
    switch (element_number)
    {
		case TRANSACTION_FORM_PARTY :
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				string = gsb_form_widget_get_name (TRANSACTION_FORM_PARTY);
				utils_set_image_with_state (GTK_WIDGET (combo), FALSE);
			}
			else
			{
				utils_set_image_with_state (GTK_WIDGET (combo), TRUE);
			}
			break;

		case TRANSACTION_FORM_CATEGORY :
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				string = gsb_form_widget_get_name (TRANSACTION_FORM_CATEGORY);
				if (gsb_form_widget_check_empty (GTK_WIDGET (combo)))
				{
					utils_set_image_with_state (GTK_WIDGET (combo), FALSE);
				}
			}
			else
			{
				utils_set_image_with_state (GTK_WIDGET (combo), TRUE);
			}
			break;

		case TRANSACTION_FORM_BUDGET :
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				string = gsb_form_widget_get_name (TRANSACTION_FORM_BUDGET);
				if (gsb_form_widget_check_empty (GTK_WIDGET (combo)))
				{
					utils_set_image_with_state (GTK_WIDGET (combo), FALSE);
				}

			}
			else
			{
				utils_set_image_with_state (GTK_WIDGET (combo), TRUE);
			}
			break;

		default :
			break;

    }

    /* if string is not NULL, the entry is empty so set the empty field to TRUE */
    if (string)
    {
        gtk_combofix_set_text (GTK_COMBOFIX (combo), _(string));
        gsb_form_widget_set_empty (GTK_WIDGET (combo), TRUE);
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
static void widget_loan_combo_account_list_changed (GtkComboBox *combo_box,
													 WidgetLoan *w_loan)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint index;
	WidgetLoanPrivate *priv;
	LoanStruct *s_loan;

	devel_debug (NULL);

	priv = widget_loan_get_instance_private (w_loan);

	s_loan = priv->s_loan;
	gsb_data_account_set_bet_init_sch_with_loan (s_loan->associated_account, FALSE);
	if (! gtk_combo_box_get_active_iter (combo_box, &iter))
		return;

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo_box));
	if (gtk_list_store_iter_is_valid ((GtkListStore *) model, &iter))
	{
		gtk_tree_model_get (model, &iter, 1, &index, -1);
		s_loan->associated_account = index;
		gsb_data_account_set_bet_init_sch_with_loan (s_loan->associated_account, s_loan->init_sch_with_loan);
	}
	else
		s_loan->associated_account = 0;
}

/**
 * create a combobox containing the list of bank accounts
 *
 * \param
 *
 * \return a new GtkCombobox containing the list of the accounts
 **/
static GtkWidget *widget_loan_combo_account_list_create (void)
{
    GSList *list_tmp;
    GtkListStore *store;
    GtkCellRenderer *renderer;
    GtkWidget *combobox;

    combobox = gtk_combo_box_new ();
    store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
	list_tmp = gsb_data_account_get_list_accounts ();

	while (list_tmp)
	{
		gint account_number;
		GtkTreeIter iter;

		account_number = gsb_data_account_get_no_account (list_tmp->data);

		if (account_number > 0
			&& !gsb_data_account_get_closed_account (account_number)
			&& gsb_data_account_get_kind (account_number) == GSB_TYPE_BANK)
		{
			gtk_list_store_append (GTK_LIST_STORE (store), &iter);
			gtk_list_store_set (store,
					 &iter,
					 0, gsb_data_account_get_name (account_number),
					 1, account_number,
					 -1);
		}
		list_tmp = list_tmp->next;
	}

    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (store));

    /* by default, this is blank, so set the first */
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

    return combobox;
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
static void widget_loan_entry_focus_in (GtkWidget *widget,
										GdkEvent *event,
										WidgetLoan *w_loan)
{
	if (GTK_IS_ENTRY (widget))
	{
		if (old_entry)
		{
			g_free (old_entry);
			old_entry = NULL;
		}
		if (gtk_entry_get_text_length (GTK_ENTRY (widget)))
		{
			old_entry = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));
		}
	}
	else if (GTK_IS_SPIN_BUTTON (widget))
	{
		old_number = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
	}
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
static void widget_loan_entry_focus_out (GtkWidget *widget,
										 GdkEvent  *event,
										 WidgetLoan *w_loan)
{
	const gchar *name;
	const gchar *tmp_str;
	gboolean valide;
	LoanStruct *s_loan;
	WidgetLoanPrivate *priv;

	devel_debug (NULL);
	priv = widget_loan_get_instance_private (w_loan);
	s_loan = priv->s_loan;
	name = gtk_widget_get_name (GTK_WIDGET (widget));

	if (g_strcmp0 (name, "entry_loan_capital") == 0)
	{
		tmp_str = gtk_entry_get_text (GTK_ENTRY (widget));
		valide = gsb_form_widget_get_valide_amout_entry (tmp_str);
		if (valide)
		{
			if (strlen (tmp_str) == 0)
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_fees_per_month),
												GTK_ENTRY_ICON_PRIMARY,
												pixbuf_NOK);
				gtk_entry_set_text (GTK_ENTRY (priv->entry_fees_per_month), "");
				s_loan->capital = 0.0;
				s_loan->amount_fees = 0.0;
			}
			else
			{
				gchar *real_str;
				GsbReal real;

				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_fees_per_month),
												GTK_ENTRY_ICON_PRIMARY,
												pixbuf_OK);
				s_loan->capital = utils_str_strtod (tmp_str, NULL);
				real = gsb_real_double_to_real (s_loan->capital);
				real_str = 	utils_real_get_string_with_currency (real, s_loan->currency_number, FALSE);
				gtk_entry_set_text (GTK_ENTRY (priv->entry_loan_capital), real_str);
				g_free (real_str);

				if (s_loan->version_number == -1)
				{
					s_loan->version_number = 0;
					s_loan->capital_du = s_loan->capital;
					bet_data_loan_add_item (s_loan);
				}
			}
		}
		else
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			gtk_entry_grab_focus_without_selecting (GTK_ENTRY (widget));
		}
	}
	else if (g_strcmp0 (name, "spinbutton_loan_duration") == 0)
	{
		gtk_spin_button_update (GTK_SPIN_BUTTON (widget));
		s_loan->duree = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (widget));
	}
	else if (g_strcmp0 (name, "spinbutton_percentage_fees") == 0)
	{
		gtk_spin_button_update (GTK_SPIN_BUTTON (widget));
		s_loan->percentage_fees = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
	}
	else if (g_strcmp0 (name, "spinbutton_rate_interest") == 0)
	{
		gtk_spin_button_update (GTK_SPIN_BUTTON (widget));
		s_loan->annual_rate = gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget));
	}
	else if (g_strcmp0 (name, "entry_fees_per_month") == 0)
	{
		tmp_str = gtk_entry_get_text (GTK_ENTRY (widget));
		valide = gsb_form_widget_get_valide_amout_entry (tmp_str);
		if (valide)
		{
			if (strlen (tmp_str) == 0)
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			else
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
				s_loan->amount_fees = utils_str_strtod (tmp_str, NULL);
				if (s_loan->type_taux < 2)
				{
					gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_fees), tmp_str);
					gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_fees),
													GTK_ENTRY_ICON_PRIMARY,
													pixbuf_NOK);
				}
			}
		}
		else
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			gtk_entry_grab_focus_without_selecting (GTK_ENTRY (widget));
		}
	}
	else if (g_strcmp0 (name, "entry_fixed_due_amount") == 0)
	{
		tmp_str = gtk_entry_get_text (GTK_ENTRY (widget));
		valide = gsb_form_widget_get_valide_amout_entry (tmp_str);
		if (valide)
		{
			if (strlen (tmp_str) == 0)
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			else
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
				s_loan->fixed_due_amount = utils_str_strtod (tmp_str, NULL);
			}
		}
		else
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			gtk_entry_grab_focus_without_selecting (GTK_ENTRY (widget));
		}
	}
	else if (g_strcmp0 (name, "form_entry") == 0)
	{
		GDate *date;

		date = gsb_date_get_last_entry_date (gtk_entry_get_text (GTK_ENTRY (widget)));
		s_loan->first_date = date;
		gsb_calendar_entry_set_date (widget, s_loan->first_date);
	}
	else if (g_strcmp0 (name, "form_entry_error") == 0)
	{
		gtk_entry_grab_focus_without_selecting (GTK_ENTRY (widget));
	}
	else if (g_strcmp0 (name, "entry_amount_first_capital") == 0)
	{
		tmp_str = gtk_entry_get_text (GTK_ENTRY (widget));
		valide = gsb_form_widget_get_valide_amout_entry (tmp_str);
		if (valide)
		{
			if (strlen (tmp_str) == 0)
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_amount_first_is_different), FALSE);
				gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_interests), "");
				s_loan->first_capital = 0.0;
				s_loan->first_interests = 0.0;
			}
			else
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
				s_loan->first_capital = utils_str_strtod (tmp_str, NULL);
			}
		}
		else
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			gtk_entry_grab_focus_without_selecting (GTK_ENTRY (widget));
		}
	}
	else if (g_strcmp0 (name, "entry_amount_first_fees") == 0)
	{
		tmp_str = gtk_entry_get_text (GTK_ENTRY (widget));
		valide = gsb_form_widget_get_valide_amout_entry (tmp_str);
		if (valide)
		{
			if (strlen (tmp_str) == 0)
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			else
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
				s_loan->first_fees = utils_str_strtod (tmp_str, NULL);
			}
		}
		else
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			gtk_entry_grab_focus_without_selecting (GTK_ENTRY (widget));
		}
	}
	else if (g_strcmp0 (name, "entry_amount_first_interests") == 0)
	{
		tmp_str = gtk_entry_get_text (GTK_ENTRY (widget));
		valide = gsb_form_widget_get_valide_amout_entry (tmp_str);
		if (valide)
		{
			if (strlen (tmp_str) == 0)
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			}
			else
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
				s_loan->first_interests = utils_str_strtod (tmp_str, NULL);
			}
		}
		else
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (widget), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			gtk_entry_grab_focus_without_selecting (GTK_ENTRY (widget));
		}
	}
	if (gsb_gui_navigation_get_current_account () == s_loan->account_number)
	{
		GtkWidget *account_page;

		account_page = grisbi_win_get_account_page ();
		bet_finance_update_amortization_tab_with_data (s_loan->account_number, account_page, s_loan);
	}
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
static void widget_loan_entry_deleted (GtkEditable *entry,
									   gint start_pos,
									   gint end_pos,
									   WidgetLoan *w_loan)
{
	if (gtk_entry_get_text_length (GTK_ENTRY (entry)) == 1)
	{
		const gchar *name;

		name = gtk_widget_get_name (GTK_WIDGET (entry));
		if (g_strcmp0 (name, "entry_loan_capital") == 0)
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (entry), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
		}
		else if (g_strcmp0 (name, "entry_fees_per_month") == 0)
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (entry), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
		}
		else if (g_strcmp0 (name, "entry_fixed_due_amount") == 0)
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (entry), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
		}
		else if (g_strcmp0 (name, "entry_amount_first_capital") == 0)
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (entry), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
		}
		else if (g_strcmp0 (name, "entry_amount_first_interests") == 0)
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (entry), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
		}
		else if (g_strcmp0 (name, "entry_amount_first_fees") == 0)
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (entry), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
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
static void widget_loan_checkbutton_amount_first_toggled (GtkToggleButton *togglebutton,
														  WidgetLoan *w_loan)
{
	WidgetLoanPrivate *priv;
	LoanStruct *s_loan;

	devel_debug (NULL);

	priv = widget_loan_get_instance_private (w_loan);
	s_loan = priv->s_loan;
	if (gtk_toggle_button_get_active (togglebutton))
	{
		gtk_widget_set_sensitive (priv->entry_amount_first_capital, TRUE);
		if (s_loan->type_taux == 2)
		{
			gtk_widget_set_sensitive (priv->entry_amount_first_fees, TRUE);
		}
		else
		{
			gchar *tmp_str;
			GsbReal real;

			real = gsb_real_double_to_real (s_loan->amount_fees);
			if (real.mantissa > 0)
			{
				tmp_str = utils_real_get_string_with_currency (gsb_real_double_to_real (s_loan->amount_fees),
															   s_loan->currency_number,
															   TRUE);
				gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_fees), tmp_str);
				g_free  (tmp_str);
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_fees),
												GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
			}
			else
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_fees),
												GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
		}
		gtk_widget_set_sensitive (priv->entry_amount_first_interests, TRUE);
		s_loan->first_is_different = TRUE;
		gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_capital),
										GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
		gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_interests),
										GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
	}
	else
	{
		gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_capital), "");
		gtk_widget_set_sensitive (priv->entry_amount_first_capital, FALSE);
		gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_capital),
										GTK_ENTRY_ICON_PRIMARY, NULL);
		gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_fees), "");
		gtk_widget_set_sensitive (priv->entry_amount_first_fees, FALSE);
		gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_fees),
										GTK_ENTRY_ICON_PRIMARY, NULL);
		gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_interests), "");
		gtk_widget_set_sensitive (priv->entry_amount_first_interests, FALSE);
		gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_interests),
										GTK_ENTRY_ICON_PRIMARY, NULL);
		s_loan->first_is_different = FALSE;
		s_loan->first_capital = 0.0;
		s_loan->first_fees = 0.0;
		s_loan->first_interests = 0.0;
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
static void widget_loan_button_amortization_table_clicked (GtkButton *button,
														   WidgetLoan *w_loan)
{
	GtkWidget *popup;
	GtkWidget *button_close;
	GtkWidget *page;
	GtkWidget *content_area;
	GtkWidget *prefs;
	GtkWidget *tree_view;
    GtkWidget *frame;
	GtkWidget *account_toolbar;
	GtkWidget *item;
	GtkWidget *label_title;
	WidgetLoanPrivate *priv;
	LoanStruct *s_loan;

	devel_debug (NULL);

	priv = widget_loan_get_instance_private (w_loan);

	s_loan = priv->s_loan;
	prefs = grisbi_win_get_prefs_dialog (NULL);
	popup = gtk_dialog_new_with_buttons (_("Amortization Table"),
										 GTK_WINDOW (prefs),
										 GTK_DIALOG_MODAL,
										 NULL, NULL,
										 NULL);
	gtk_widget_show (popup);

	button_close = gtk_button_new_with_label (_("Close"));
	gtk_dialog_add_action_widget (GTK_DIALOG (popup), button_close, GTK_RESPONSE_NO);

	/* set data of loan */
	page = bet_finance_ui_create_amortization_heading (popup);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (popup));
	gtk_box_pack_start (GTK_BOX (content_area), page, TRUE, TRUE, 0);

	/* création de la liste des données */
	label_title = g_object_get_data (G_OBJECT (popup), "label_title");
	if (s_loan->invers_cols_cap_ech)
    	tree_view = bet_finance_ui_create_amortization_tree_view (page, SPP_ORIGIN_INVERSE_FINANCE);
	else
    	tree_view = bet_finance_ui_create_amortization_tree_view (page, SPP_ORIGIN_FINANCE);

    g_object_set_data (G_OBJECT (tree_view), "origin", GINT_TO_POINTER (SPP_ORIGIN_CONFIG));
    g_object_set_data (G_OBJECT (popup), "bet_finance_tree_view", tree_view);
    g_object_set_data (G_OBJECT (tree_view), "label_title", label_title);

	/* demarre le tableau à la première échéance */
	g_object_set_data (G_OBJECT (tree_view), "amortization_initial_date", GINT_TO_POINTER (TRUE));

	/* set toolbar */
	frame = g_object_get_data (G_OBJECT (popup), "frame");
	account_toolbar = bet_finance_ui_create_simulator_toolbar (page, tree_view, FALSE, FALSE);
	item = g_object_get_data (G_OBJECT (tree_view), "amortization_initial_date_button");
	gtk_widget_set_sensitive (item, FALSE);
    gtk_container_add (GTK_CONTAINER (frame), account_toolbar);

	gtk_widget_set_size_request (popup, 800, 600);
	gtk_window_set_position (GTK_WINDOW (popup), GTK_WIN_POS_CENTER_ALWAYS);
	gtk_widget_show_all (page);

	/* init the tab */
	bet_finance_update_amortization_tab_with_data (s_loan->account_number, popup, s_loan);

	gtk_dialog_run (GTK_DIALOG (popup));
	gtk_widget_destroy (popup);
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
static void widget_loan_radiobutton_type_taux_toggled (GtkToggleButton *togglebutton,
													   WidgetLoan *w_loan)
{
	const gchar *name;
	WidgetLoanPrivate *priv;
	LoanStruct *s_loan;

	priv = widget_loan_get_instance_private (w_loan);
	name = gtk_widget_get_name (GTK_WIDGET (togglebutton));
	s_loan = priv->s_loan;

	if (g_strcmp0 (name, "bet_config_type_taux_3") == 0)
		s_loan->type_taux = 2;
	else if (g_strcmp0 (name, "bet_config_type_taux_2") == 0)
		s_loan->type_taux = 1;
	else
		s_loan->type_taux = 0;

	/* adapt display */
	widget_loan_adapt_display_from_type_taux (s_loan->type_taux, priv);

	if (gsb_gui_navigation_get_current_account () == s_loan->account_number)
	{
		GtkWidget *account_page;

		account_page = grisbi_win_get_account_page ();
		if (gtk_notebook_get_current_page (GTK_NOTEBOOK (account_page)) == GSB_FINANCE_PAGE)
			bet_finance_update_amortization_tab_with_data (s_loan->account_number, account_page, s_loan);
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static DialogScheduled *widget_loan_dialog_scheduled_init (GtkWidget *page,
														   WidgetLoan *w_loan)
{
	GtkWidget *entry;
	GtkWidget *label;
	GtkWidget *paddingbox;
	GtkWidget *hbox;
	GtkWidget *separator;
	GtkWidget *icon;
	GSList *tmp_list;
	gchar *tmp_str;
	const gchar *text_frequency [] = {N_("Monthly"), N_("Quarterly"), N_("Yearly"), NULL};
	gint tmp_number = 0;
	gint tmp_number2 = 0;
	gint scheduled_number = 0;
	GrisbiWinEtat *w_etat;
	DialogScheduled *s_sch_dialog;
	LoanStruct *s_loan;
	WidgetLoanPrivate *priv;

	devel_debug (NULL);
	priv = widget_loan_get_instance_private (w_loan);
	w_etat = grisbi_win_get_w_etat ();

	s_loan = priv->s_loan;
	s_sch_dialog = widget_loan_scheduled_dialog_struct_init ();

	/* Main account */
	paddingbox = new_paddingbox_with_title (page, FALSE, _("Associated account"));

	/* set associated account */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
	gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, FALSE, 0);

	label = gtk_label_new (_("Associated account: "));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	s_sch_dialog->AccountListCombo = widget_loan_combo_account_list_create ();
	gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->AccountListCombo, FALSE, FALSE, 0);

	if (s_loan->associated_account)
		gsb_combo_box_set_index (s_sch_dialog->AccountListCombo, s_loan->associated_account);
	else
		s_loan->associated_account = gsb_combo_box_get_index (s_sch_dialog->AccountListCombo);
	gsb_data_account_set_bet_init_sch_with_loan (s_loan->associated_account, s_loan->init_sch_with_loan);

	scheduled_number = s_loan->associated_scheduled;

	/* set frequency */
	label = gtk_label_new (_("Frequency: "));
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	s_sch_dialog->AFreqencyCombo = gsb_combo_box_new_with_index (text_frequency, NULL, NULL);
	gtk_widget_show (s_sch_dialog->AFreqencyCombo);
	gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->AFreqencyCombo, FALSE, FALSE, 0);

	gsb_combo_box_set_index (s_sch_dialog->AFreqencyCombo, s_loan->associated_frequency);

	/* add separator */
	separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_margin_bottom (separator, MARGIN_BOTTOM);
	gtk_widget_set_margin_top (separator, MARGIN_TOP);
	gtk_box_pack_start (GTK_BOX (paddingbox), separator, TRUE, TRUE, 0);

	/* Mother transaction */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, TRUE, 0);

	label = gtk_label_new (_("Mother transaction:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	/* création de la boite de sélection du tiers */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, TRUE, 0);

    icon = utils_get_image_with_state (GTK_MESSAGE_ERROR, FALSE, _("This field is required"), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

	tmp_list = gsb_data_payee_get_name_and_report_list ();
	s_sch_dialog->APayeeCombo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_payee,
											  w_etat->combofix_case_sensitive,
											  FALSE,
											  METATREE_PAYEE);
	gsb_data_payee_free_name_and_report_list (tmp_list);
    gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->APayeeCombo, TRUE, TRUE, 0);
    g_object_set_data (G_OBJECT (s_sch_dialog->APayeeCombo), "icon", icon);
	g_object_set_data (G_OBJECT (s_sch_dialog->APayeeCombo), "element", GINT_TO_POINTER (TRANSACTION_FORM_PARTY));

	if (scheduled_number)
	{
		tmp_number = gsb_data_scheduled_get_payee_number (scheduled_number);
		gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->APayeeCombo),
											  gsb_data_payee_get_name (tmp_number, FALSE));
		utils_set_image_with_state (GTK_WIDGET (s_sch_dialog->APayeeCombo), TRUE);
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->APayeeCombo), _("Payee"));
		gsb_form_widget_set_empty (s_sch_dialog->APayeeCombo, TRUE);
	}

	/* set Catégorie */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, TRUE, TRUE, 0);

    icon = utils_get_image_with_state (GTK_MESSAGE_ERROR, TRUE, _("One of the two fields is required"), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
	s_sch_dialog->ACategCombo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_CATEGORY);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->ACategCombo, TRUE, TRUE, 0);

	gsb_form_widget_set_empty (s_sch_dialog->ACategCombo, FALSE);

	/* select Split of transaction in list */
	tmp_str = g_strdup (_("Split of transaction"));
    gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->ACategCombo), tmp_str);
	g_free (tmp_str);
	gtk_widget_set_sensitive (s_sch_dialog->ACategCombo, FALSE);
    g_object_set_data (G_OBJECT (s_sch_dialog->ACategCombo), "icon", icon);
	g_object_set_data (G_OBJECT (s_sch_dialog->ACategCombo), "element", GINT_TO_POINTER (TRANSACTION_FORM_CATEGORY));

    /* Création du mode de payement "Direct debit" si nécessaire*/
	if (scheduled_number)
	{
		tmp_number = gsb_data_scheduled_get_method_of_payment_number (scheduled_number);
	}
	else
	{
		tmp_number = gsb_data_payment_get_number_by_name (_("Direct debit"), s_loan->associated_account);
		if (tmp_number == 0)
		{
			tmp_number = gsb_data_payment_new (_("Direct debit"));
			gsb_data_payment_set_account_number (tmp_number, s_loan->account_number);
		}
	}
	s_sch_dialog->APaymentCombo = gtk_combo_box_new ();
    gsb_payment_method_create_combo_list (s_sch_dialog->APaymentCombo, GSB_PAYMENT_DEBIT, s_loan->associated_account, 0, FALSE);
	gsb_payment_method_set_payment_position (s_sch_dialog->APaymentCombo, tmp_number);
    gtk_widget_set_tooltip_text (GTK_WIDGET (s_sch_dialog->APaymentCombo), _("Choose the method of payment"));
    gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->APaymentCombo, FALSE, FALSE, 0);

	/* add separator */
	separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_margin_bottom (separator, MARGIN_BOTTOM);
	gtk_widget_set_margin_top (separator, MARGIN_TOP);
	gtk_box_pack_start (GTK_BOX (paddingbox), separator, TRUE, TRUE, 0);

	/* First split transaction: Capital */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, TRUE, 0);

	label = gtk_label_new (_("First split transaction: Capital"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	/* set Catégorie */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, FALSE, 0);

    icon = utils_get_image_with_state (GTK_MESSAGE_ERROR, TRUE, _("One of the two fields is required"), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
	s_sch_dialog->FCategCombo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_CATEGORY);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->FCategCombo, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (s_sch_dialog->AccountListCombo), "combo", s_sch_dialog->FCategCombo);
    g_object_set_data (G_OBJECT (s_sch_dialog->FCategCombo), "icon", icon);
	g_object_set_data (G_OBJECT (s_sch_dialog->FCategCombo), "element",
					   GINT_TO_POINTER (TRANSACTION_FORM_CATEGORY));

	if (scheduled_number)
	{
		gsb_form_widget_set_empty (s_sch_dialog->FCategCombo, FALSE);
		/* select the transfer in list */
		tmp_number = gsb_data_scheduled_get_account_number_transfer (scheduled_number+1);

		tmp_str = g_strconcat (_("Transfer : "),
							   gsb_data_account_get_name (tmp_number),
							   NULL);
	}
	else
	{
		tmp_str = g_strconcat (_("Transfer : "),
							   gsb_data_account_get_name (s_loan->account_number),
							   NULL);
	}
    gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->FCategCombo), tmp_str);
	g_free (tmp_str);
	gtk_widget_set_sensitive (s_sch_dialog->FCategCombo, FALSE);

	/* saisie IB */
	tmp_list = gsb_data_budget_get_name_list (TRUE, TRUE);
	s_sch_dialog->FBudgetCombo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_BUDGET);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->FBudgetCombo, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (s_sch_dialog->FBudgetCombo), "icon", icon);
	g_object_set_data (G_OBJECT (s_sch_dialog->FBudgetCombo),
					   "element",
					   GINT_TO_POINTER (TRANSACTION_FORM_BUDGET));

	if (scheduled_number)
	{
		tmp_number = gsb_data_scheduled_get_budgetary_number (scheduled_number+1);
		if (tmp_number)
		{
			tmp_number2 = gsb_data_scheduled_get_sub_budgetary_number (scheduled_number+1);
			tmp_str = gsb_data_budget_get_name (tmp_number, tmp_number2, NULL);
			gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->FBudgetCombo), tmp_str);
			g_free(tmp_str);
			utils_set_image_with_state (GTK_WIDGET (s_sch_dialog->FBudgetCombo), TRUE);
		}
		else
		{
			gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->FBudgetCombo), _("Budgetary line"));
			gsb_form_widget_set_empty (s_sch_dialog->FBudgetCombo, TRUE);
		}
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->FBudgetCombo), _("Budgetary line"));
		gsb_form_widget_set_empty (s_sch_dialog->FBudgetCombo, TRUE);
	}

	/* add separator */
	separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_margin_bottom (separator, MARGIN_BOTTOM);
	gtk_widget_set_margin_top (separator, MARGIN_TOP);
	gtk_box_pack_start (GTK_BOX (paddingbox), separator, TRUE, TRUE, 0);

	/* Second split transaction: Interests */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, TRUE, 0);

	label = gtk_label_new (_("Second split transaction: Interests"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	/* Catégorie */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, TRUE, 0);

    icon = utils_get_image_with_state (GTK_MESSAGE_ERROR, FALSE, _("One of the two fields is required"), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
	s_sch_dialog->SCategCombo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_CATEGORY);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->SCategCombo, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (s_sch_dialog->SCategCombo), "icon", icon);
	g_object_set_data (G_OBJECT (s_sch_dialog->SCategCombo),
					   "element",
					   GINT_TO_POINTER (TRANSACTION_FORM_CATEGORY));

	if (scheduled_number)
	{
		tmp_number = gsb_data_scheduled_get_category_number (scheduled_number+2);
		if (tmp_number)
		{
			tmp_number2 = gsb_data_scheduled_get_sub_category_number (scheduled_number+2);
			tmp_str = gsb_data_category_get_name (tmp_number, tmp_number2, NULL);
			gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->SCategCombo), tmp_str);
			g_free(tmp_str);
			utils_set_image_with_state (GTK_WIDGET (s_sch_dialog->SCategCombo), TRUE);
		}
		else
		{
			gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->SCategCombo), _("Categories : Sub-categories"));
			gsb_form_widget_set_empty (s_sch_dialog->SCategCombo, TRUE);
		}
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->SCategCombo), _("Categories : Sub-categories"));
		gsb_form_widget_set_empty (s_sch_dialog->SCategCombo, TRUE);
	}

	/* saisie (sous)IB */
	tmp_list = gsb_data_budget_get_name_list (TRUE, TRUE);
	s_sch_dialog->SBudgetCombo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_BUDGET);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->SBudgetCombo, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (s_sch_dialog->SBudgetCombo), "icon", icon);
	g_object_set_data (G_OBJECT (s_sch_dialog->SBudgetCombo),
					   "element",
					   GINT_TO_POINTER (TRANSACTION_FORM_BUDGET));

	if (scheduled_number)
	{
		tmp_number = gsb_data_scheduled_get_budgetary_number (scheduled_number+2);
		if (tmp_number)
		{
			tmp_number2 = gsb_data_scheduled_get_sub_budgetary_number (scheduled_number+2);
			tmp_str = gsb_data_budget_get_name (tmp_number, tmp_number2, NULL);
			gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->SBudgetCombo), tmp_str);
			g_free(tmp_str);
			utils_set_image_with_state (GTK_WIDGET (s_sch_dialog->SBudgetCombo), TRUE);
		}
		else
		{
			gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->SBudgetCombo), _("Budgetary line"));
			gsb_form_widget_set_empty (s_sch_dialog->SBudgetCombo, TRUE);
		}
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->SBudgetCombo), _("Budgetary line"));
		gsb_form_widget_set_empty (s_sch_dialog->SBudgetCombo, TRUE);
	}

	/* add separator */
	separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_margin_bottom (separator, MARGIN_BOTTOM);
	gtk_widget_set_margin_top (separator, MARGIN_TOP);
	gtk_box_pack_start (GTK_BOX (paddingbox), separator, TRUE, TRUE, 0);

	/* Third split transaction : Fees */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, TRUE, 0);

	label = gtk_label_new (_("Third split transaction: Fees"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	/* Catégorie */
    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
    gtk_box_pack_start (GTK_BOX (paddingbox), hbox, FALSE, TRUE, 0);

    icon = utils_get_image_with_state (GTK_MESSAGE_ERROR, FALSE, _("One of the two fields is required"), NULL);
    gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
	s_sch_dialog->TCategCombo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_CATEGORY);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->TCategCombo, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (s_sch_dialog->TCategCombo), "icon", icon);
	g_object_set_data (G_OBJECT (s_sch_dialog->TCategCombo),
					   "element",
					   GINT_TO_POINTER (TRANSACTION_FORM_CATEGORY));
	if (scheduled_number)
	{
		tmp_number = gsb_data_scheduled_get_category_number (scheduled_number+3);
		if (tmp_number)
		{
			tmp_number2 = gsb_data_scheduled_get_sub_category_number (scheduled_number+3);
			tmp_str = gsb_data_category_get_name (tmp_number, tmp_number2, NULL);
			gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->TCategCombo), tmp_str);
			g_free(tmp_str);
			utils_set_image_with_state (GTK_WIDGET (s_sch_dialog->TCategCombo), TRUE);
		}
		else
		{
			gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->TCategCombo), _("Categories : Sub-categories"));
			gsb_form_widget_set_empty (s_sch_dialog->TCategCombo, TRUE);
		}
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->TCategCombo), _("Categories : Sub-categories"));
		gsb_form_widget_set_empty (s_sch_dialog->TCategCombo, TRUE);
	}

	/* saisie (sous)IB */
	tmp_list = gsb_data_budget_get_name_list (TRUE, TRUE);
	s_sch_dialog->TBudgetCombo = gtk_combofix_new_with_properties (tmp_list,
											  w_etat->combofix_force_category,
											  w_etat->combofix_case_sensitive,
											  w_etat->combofix_mixed_sort,
											  METATREE_BUDGET);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_box_pack_start (GTK_BOX (hbox), s_sch_dialog->TBudgetCombo, FALSE, FALSE, 0);
    g_object_set_data (G_OBJECT (s_sch_dialog->TBudgetCombo), "icon", icon);
	g_object_set_data (G_OBJECT (s_sch_dialog->TBudgetCombo),
					   "element",
					   GINT_TO_POINTER (TRANSACTION_FORM_BUDGET));

	if (scheduled_number)
	{
		tmp_number = gsb_data_scheduled_get_budgetary_number (scheduled_number+3);
		if (tmp_number)
		{
			tmp_number2 = gsb_data_scheduled_get_sub_budgetary_number (scheduled_number+3);
			tmp_str = gsb_data_budget_get_name (tmp_number, tmp_number2, NULL);
			gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->TBudgetCombo), tmp_str);
			g_free(tmp_str);
			utils_set_image_with_state (GTK_WIDGET (s_sch_dialog->TBudgetCombo), TRUE);
		}
		else
		{
			gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->TBudgetCombo), _("Budgetary line"));
			gsb_form_widget_set_empty (s_sch_dialog->TBudgetCombo, TRUE);
		}
	}
	else
	{
		gtk_combofix_set_text (GTK_COMBOFIX (s_sch_dialog->TBudgetCombo), _("Budgetary line"));
		gsb_form_widget_set_empty (s_sch_dialog->TBudgetCombo, TRUE);
	}

	/* set others signals */
	/* callback for combo_account */
    g_signal_connect (s_sch_dialog->AccountListCombo,
					  "changed",
					  G_CALLBACK (widget_loan_combo_account_list_changed),
					  w_loan);

	/* callback for first combo data budget */
	entry = gtk_combofix_get_entry (GTK_COMBOFIX (s_sch_dialog->FBudgetCombo));
	g_signal_connect (G_OBJECT (entry),
					  "focus-in-event",
					  G_CALLBACK (widget_loan_combofix_enter_focus),
					  NULL);
    g_signal_connect (G_OBJECT (entry),
					  "focus-out-event",
					  G_CALLBACK (widget_loan_combofix_lose_focus),
					  s_sch_dialog->FBudgetCombo);

	/* callback for second combo data budget */
	entry = gtk_combofix_get_entry (GTK_COMBOFIX (s_sch_dialog->SBudgetCombo));
	g_signal_connect (G_OBJECT (entry),
					  "focus-in-event",
					  G_CALLBACK (widget_loan_combofix_enter_focus),
					  NULL);
    g_signal_connect (G_OBJECT (entry),
					  "focus-out-event",
					  G_CALLBACK (widget_loan_combofix_lose_focus),
					  s_sch_dialog->SBudgetCombo);

	/* callback for third combo data budget */
	entry = gtk_combofix_get_entry (GTK_COMBOFIX (s_sch_dialog->TBudgetCombo));
	g_signal_connect (G_OBJECT (entry),
					  "focus-in-event",
					  G_CALLBACK (widget_loan_combofix_enter_focus),
					  NULL);
    g_signal_connect (G_OBJECT (entry),
					  "focus-out-event",
					  G_CALLBACK (widget_loan_combofix_lose_focus),
					  s_sch_dialog->TBudgetCombo);

	/* callback for second combo data catégories */
	entry = gtk_combofix_get_entry (GTK_COMBOFIX (s_sch_dialog->SCategCombo));
	g_signal_connect (G_OBJECT (entry),
					  "focus-in-event",
					  G_CALLBACK (widget_loan_combofix_enter_focus),
					  NULL);
    g_signal_connect (G_OBJECT (entry),
					  "focus-out-event",
					  G_CALLBACK (widget_loan_combofix_lose_focus),
					  s_sch_dialog->SCategCombo);

	/* callback for third combo data catégories */
	entry = gtk_combofix_get_entry (GTK_COMBOFIX (s_sch_dialog->TCategCombo));
	g_signal_connect (G_OBJECT (entry),
					  "focus-in-event",
					  G_CALLBACK (widget_loan_combofix_enter_focus),
					  NULL);
    g_signal_connect (G_OBJECT (entry),
					  "focus-out-event",
					  G_CALLBACK (widget_loan_combofix_lose_focus),
					  s_sch_dialog->TCategCombo);

	/* callback for combo data payees */
	entry = gtk_combofix_get_entry (GTK_COMBOFIX (s_sch_dialog->APayeeCombo));
	g_signal_connect (G_OBJECT (entry),
					  "focus-in-event",
					  G_CALLBACK (widget_loan_combofix_enter_focus),
					  NULL);
    g_signal_connect (G_OBJECT (entry),
					  "focus-out-event",
					  G_CALLBACK (widget_loan_combofix_lose_focus),
					  s_sch_dialog->APayeeCombo);

	return s_sch_dialog;
}

static void widget_loan_button_delete_scheduled_clicked (GtkButton *button,
														 WidgetLoan *w_loan)
{
	LoanStruct *s_loan;
	WidgetLoanPrivate *priv;

	devel_debug (NULL);

	priv = widget_loan_get_instance_private (w_loan);
	s_loan = priv->s_loan;
	gsb_data_scheduled_remove_scheduled (s_loan->associated_scheduled);

	/* change button_init_scheduled */
	gtk_button_set_label (GTK_BUTTON (priv->button_init_scheduled), _("Create new scheduled transaction"));
	g_signal_handlers_disconnect_by_func(priv->button_init_scheduled,
										 G_CALLBACK (widget_loan_button_delete_scheduled_clicked),
										 w_loan);
	g_signal_connect (priv->button_init_scheduled,
					  "clicked",
					  G_CALLBACK (widget_loan_button_init_scheduled_clicked),
					  w_loan);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_init_sch_with_loan), FALSE);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_loan_button_init_scheduled_clicked (GtkButton *button,
													   WidgetLoan *w_loan)
{
	GtkWidget *popup;
	GtkWidget *button_cancel;
	GtkWidget *button_OK;
	GtkWidget *page;
	GtkWidget *content_area;
	GtkWidget *prefs;
	gint result;
	DialogScheduled *s_sch_dialog;
	LoanStruct *s_loan;
	WidgetLoanPrivate *priv;

	devel_debug (NULL);

	priv = widget_loan_get_instance_private (w_loan);
	s_loan = priv->s_loan;
	prefs = grisbi_win_get_prefs_dialog (NULL);

	popup = gtk_dialog_new_with_buttons (_("Create new scheduled transaction"),
										 GTK_WINDOW (prefs),
										 GTK_DIALOG_MODAL,
										 NULL, NULL,
										 NULL);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (popup), button_cancel, GTK_RESPONSE_CANCEL);
	gtk_widget_set_can_default (button_cancel, TRUE);

	button_OK = gtk_button_new_with_label (_("Validate"));
	gtk_dialog_add_action_widget (GTK_DIALOG (popup), button_OK, GTK_RESPONSE_OK);
	gtk_widget_set_can_default (button_OK, TRUE);

	page = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	content_area = gtk_dialog_get_content_area (GTK_DIALOG (popup));
	gtk_box_pack_start (GTK_BOX (content_area), page, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (page), BOX_BORDER_WIDTH);

	/* initialisation de capital du */
	s_loan->capital_du = s_loan->capital;

	/* initialisation des widgets */
	s_sch_dialog = widget_loan_dialog_scheduled_init (page, w_loan);

	gtk_widget_show_all (popup);

dialog_return:
	result = gtk_dialog_run (GTK_DIALOG (popup));
	if (result == GTK_RESPONSE_OK)
    {
		GDate *tmp_date;
		const gchar *text;
		gint associated_frequency;
		gint associated_payee;
		gint associated_payment;
		gint scheduled_number;
		gint tmp_number = 0;
		AmortissementStruct *s_amortissement;
		GsbReal amount;

		/* set mother scheduled transaction */
		/* set data for scheduled transaction */
		scheduled_number = gsb_data_scheduled_new_scheduled ();
		s_loan->associated_scheduled = scheduled_number;
		/* set split_scheduled */
		gsb_data_scheduled_set_split_of_scheduled (scheduled_number, TRUE);
		/* set account number */
		gsb_data_scheduled_set_account_number (scheduled_number, s_loan->associated_account);
		/* set automatic */
		gsb_data_scheduled_set_automatic_scheduled (scheduled_number, TRUE);
		/* set date */
		gsb_data_scheduled_set_date (scheduled_number, s_loan->first_date);
		/* set frequency */
		s_loan->associated_frequency = gsb_combo_box_get_index (s_sch_dialog->AFreqencyCombo);
		associated_frequency = widget_loan_combo_frequency_get_frequency (s_loan->associated_frequency);
		gsb_data_scheduled_set_frequency (scheduled_number, associated_frequency);	/* par mois par défaut */
		/* set currency */
		gsb_data_scheduled_set_currency_number (scheduled_number, s_loan->currency_number);
		/* set final_date */
		tmp_date = gsb_date_copy (s_loan->first_date);
		g_date_add_months (tmp_date, s_loan->duree-1);
		gsb_data_scheduled_set_limit_date (scheduled_number, tmp_date);
		g_date_free (tmp_date);
		/* set amount */
		s_amortissement = bet_finance_get_echeance_first (s_loan, s_loan->first_date);
		amount = gsb_real_opposite (gsb_real_double_to_real (s_amortissement->echeance));
		gsb_data_scheduled_set_amount (scheduled_number, amount);
		/* set payee_number */
		text = gtk_combofix_get_text (GTK_COMBOFIX (s_sch_dialog->APayeeCombo));
		associated_payee = gsb_data_payee_get_number_by_name (text, TRUE);
		gsb_data_scheduled_set_payee_number (scheduled_number, associated_payee);
		/* set payment_number */
		associated_payment = gsb_payment_method_get_selected_number (s_sch_dialog->APaymentCombo);
		gsb_data_scheduled_set_method_of_payment_number (scheduled_number, associated_payment);

		/* set first child : capital repaid and transfer */
		/* set data for first child */
		scheduled_number = gsb_data_scheduled_new_scheduled_with_number (s_loan->associated_scheduled+1);
		/* set mother */
		gsb_data_scheduled_set_mother_scheduled_number (scheduled_number, s_loan->associated_scheduled);
		/* set account number */
		gsb_data_scheduled_set_account_number (scheduled_number, s_loan->associated_account);
		/* set automatic */
		gsb_data_scheduled_set_automatic_scheduled (scheduled_number, TRUE);
		/* set date */
		gsb_data_scheduled_set_date (scheduled_number, s_loan->first_date);
		/* set frequency */
		gsb_data_scheduled_set_frequency (scheduled_number, associated_frequency);	/* par mois par défaut */
		/* set transfert account number */
		gsb_data_scheduled_set_account_number_transfer (scheduled_number, s_loan->account_number);
		/* set currency */
		gsb_data_scheduled_set_currency_number (scheduled_number, s_loan->currency_number);
		/* set amount */
		amount = gsb_real_opposite (gsb_real_double_to_real (s_amortissement->principal));
		gsb_data_scheduled_set_amount (scheduled_number, amount);
		/* set payee_number */
		gsb_data_scheduled_set_payee_number (scheduled_number, associated_payee);
		/* set payment_number */
		gsb_data_scheduled_set_method_of_payment_number (scheduled_number, associated_payment);
		/* set budgetary_number */
		if (!gsb_form_widget_check_empty (s_sch_dialog->FBudgetCombo))
		{
			text = gtk_combofix_get_text (GTK_COMBOFIX (s_sch_dialog->FBudgetCombo));
			gsb_data_budget_set_budget_from_string (scheduled_number, text, FALSE);
		}
		/* set contra payment_number */
		tmp_number = gsb_data_payment_get_transfer_payment_number (s_loan->account_number);
		gsb_data_scheduled_set_contra_method_of_payment_number (scheduled_number, tmp_number);

		/* set second child : interests */
		/* set data for second child */
		scheduled_number = gsb_data_scheduled_new_scheduled_with_number (s_loan->associated_scheduled+2);
		/* set mother */
		gsb_data_scheduled_set_mother_scheduled_number (scheduled_number, s_loan->associated_scheduled);
		/* set account number */
		gsb_data_scheduled_set_account_number (scheduled_number, s_loan->associated_account);
		/* set automatic */
		gsb_data_scheduled_set_automatic_scheduled (scheduled_number, TRUE);
		/* set date */
		gsb_data_scheduled_set_date (scheduled_number, s_loan->first_date);
		/* set frequency */
		gsb_data_scheduled_set_frequency (scheduled_number, associated_frequency);	/* par mois par défaut */
		/* set currency */
		gsb_data_scheduled_set_currency_number (scheduled_number, s_loan->currency_number);
		/* set amount */
		amount = gsb_real_opposite (gsb_real_double_to_real (s_amortissement->interets));
		gsb_data_scheduled_set_amount (scheduled_number, amount);
		/* set payee_number */
		gsb_data_scheduled_set_payee_number (scheduled_number, associated_payee);
		/* set payment_number */
		gsb_data_scheduled_set_method_of_payment_number (scheduled_number, associated_payment);
		/* set budgetary_number */
		if (!gsb_form_widget_check_empty (s_sch_dialog->SBudgetCombo))
		{
			text = gtk_combofix_get_text (GTK_COMBOFIX (s_sch_dialog->SBudgetCombo));
			gsb_data_budget_set_budget_from_string (scheduled_number, text, FALSE);
		}
		/* set category_number */
		if (!gsb_form_widget_check_empty (s_sch_dialog->SCategCombo))
		{
			text = gtk_combofix_get_text (GTK_COMBOFIX (s_sch_dialog->SCategCombo));
			gsb_data_category_set_category_from_string (scheduled_number, text, FALSE);
		}
		/* set third child : interests */
		/* set data for third child */
		scheduled_number = gsb_data_scheduled_new_scheduled_with_number (s_loan->associated_scheduled+3);
		/* set mother */
		gsb_data_scheduled_set_mother_scheduled_number (scheduled_number, s_loan->associated_scheduled);
		/* set account number */
		gsb_data_scheduled_set_account_number (scheduled_number, s_loan->associated_account);
		/* set automatic */
		gsb_data_scheduled_set_automatic_scheduled (scheduled_number, TRUE);
		/* set date */
		gsb_data_scheduled_set_date (scheduled_number, s_loan->first_date);
		/* set frequency */
		gsb_data_scheduled_set_frequency (scheduled_number, associated_frequency);	/* par mois par défaut */
		/* set currency */
		gsb_data_scheduled_set_currency_number (scheduled_number, s_loan->currency_number);
		/* set amount */
		amount = gsb_real_opposite (gsb_real_double_to_real (s_amortissement->frais));
		gsb_data_scheduled_set_amount (scheduled_number, amount);
		/* set payee_number */
		gsb_data_scheduled_set_payee_number (scheduled_number, associated_payee);
		/* set payment_number */
		gsb_data_scheduled_set_method_of_payment_number (scheduled_number, associated_payment);
		/* set budgetary_number */
		if (!gsb_form_widget_check_empty (s_sch_dialog->TBudgetCombo))
		{
			text = gtk_combofix_get_text (GTK_COMBOFIX (s_sch_dialog->TBudgetCombo));
			gsb_data_budget_set_budget_from_string (scheduled_number, text, FALSE);
		}
		/* set category_number */
		if (!gsb_form_widget_check_empty (s_sch_dialog->TCategCombo))
		{
			text = gtk_combofix_get_text (GTK_COMBOFIX (s_sch_dialog->TCategCombo));
			gsb_data_category_set_category_from_string (scheduled_number, text, FALSE);
		}

		bet_data_finance_structure_amortissement_free(s_amortissement);
		if (!widget_loan_scheduled_dialog_validate (s_sch_dialog))
		{
			goto dialog_return;
		}
		gtk_button_set_label (GTK_BUTTON (priv->button_init_scheduled), _("Delete scheduled transaction"));
		g_signal_handlers_disconnect_by_func(priv->button_init_scheduled,
											 G_CALLBACK (widget_loan_button_init_scheduled_clicked),
											 w_loan);
		g_signal_connect (priv->button_init_scheduled,
						  "clicked",
						  G_CALLBACK (widget_loan_button_delete_scheduled_clicked),
						  w_loan);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_init_sch_with_loan), FALSE);
	}
	widget_loan_scheduled_dialog_struct_free (s_sch_dialog);
	gtk_widget_destroy (popup);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_loan_checkbutton_init_sch_with_loan_toggle (GtkToggleButton *togglebutton,
															   WidgetLoan *w_loan)
{
	WidgetLoanPrivate *priv;
	LoanStruct *s_loan;

	devel_debug (NULL);
	priv = widget_loan_get_instance_private (w_loan);

	s_loan = priv->s_loan;
	s_loan->init_sch_with_loan = gtk_toggle_button_get_active (togglebutton);
	gsb_data_account_set_bet_init_sch_with_loan (s_loan->account_number, s_loan->init_sch_with_loan);
	gtk_widget_set_sensitive (priv->button_init_scheduled, s_loan->init_sch_with_loan);
	if (!s_loan->init_sch_with_loan)
	{
		/* change button_init_scheduled */
		gtk_button_set_label (GTK_BUTTON (priv->button_init_scheduled), _("Create new scheduled transaction"));
		g_signal_handlers_disconnect_by_func(priv->button_init_scheduled,
											 G_CALLBACK (widget_loan_button_delete_scheduled_clicked),
											 w_loan);
		g_signal_connect (priv->button_init_scheduled,
						  "clicked",
						  G_CALLBACK (widget_loan_button_init_scheduled_clicked),
						  w_loan);
		s_loan->associated_account = 0;
		s_loan->associated_scheduled = 0;
		s_loan->associated_frequency = 0;
		s_loan->capital_du = 0.0;
	}
}

/**
 * Création du widget de gestion des prêts
 *
 * \param loan
 *
 * \return
 **/
static void widget_loan_setup_widget (WidgetLoan *w_loan,
									  LoanStruct *s_loan)
{
	GtkWidget *separator;
	gchar *tmp_str = NULL;
	gint devise;
	GsbReal real;
	WidgetLoanPrivate *priv;

	devel_debug (NULL);
	priv = widget_loan_get_instance_private (w_loan);

	/* set s_loan structure */
	priv->s_loan = s_loan;

	/* set widgets */

	/* fix geometry with low_definition_screen variable*/
	separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	if (grisbi_app_get_low_definition_screen ())
	{
		gtk_grid_attach (GTK_GRID (priv->grid_loan_data), priv->checkbutton_init_sch_with_loan, 0, 8, 6, 1);
		gtk_grid_attach (GTK_GRID (priv->grid_loan_data), priv->button_init_scheduled, 0, 9, 6, 1);
		gtk_grid_attach (GTK_GRID (priv->grid_loan_data), separator, 0, 10, 6, 1);
		gtk_grid_attach (GTK_GRID (priv->grid_loan_data), priv->checkbutton_invers_cols_cap_ech, 0, 11, 6, 1);
		gtk_grid_attach (GTK_GRID (priv->grid_loan_data), priv->button_amortization_table, 0, 12, 6, 1);
		gtk_widget_show (separator);
	}
	else
	{
		gtk_grid_attach (GTK_GRID (priv->grid_loan_data), priv->checkbutton_init_sch_with_loan, 0, 7, 4, 1);
		gtk_grid_attach (GTK_GRID (priv->grid_loan_data), priv->button_init_scheduled, 4, 7, 2, 1);
		gtk_grid_attach (GTK_GRID (priv->grid_loan_data), separator, 0, 9, 6, 1);
		gtk_grid_attach (GTK_GRID (priv->grid_loan_data), priv->checkbutton_invers_cols_cap_ech, 0, 10, 6, 1);
		gtk_grid_attach (GTK_GRID (priv->grid_loan_data), priv->button_amortization_table, 0, 11, 6, 1);
		gtk_widget_show (separator);
	}

	/* Date of first Repayment */
	gsb_calendar_entry_new_from_ui (priv->entry_first_date, FALSE);

	if (s_loan)
	{
		/* set devises */
		devise = gsb_data_account_get_currency (s_loan->account_number);
		priv->code_devise = g_strdup (gsb_data_currency_get_nickname_or_code_iso (devise));
		gtk_label_set_text (GTK_LABEL (priv->label_fees_per_month_devise), priv->code_devise);
		gtk_label_set_text (GTK_LABEL (priv->label_first_capital_devise), priv->code_devise);
		gtk_label_set_text (GTK_LABEL (priv->label_first_fees_devise), priv->code_devise);
		gtk_label_set_text (GTK_LABEL (priv->label_first_interests_devise), priv->code_devise);
		gtk_label_set_text (GTK_LABEL (priv->label_fixed_amount_devise), priv->code_devise);
		gtk_label_set_text (GTK_LABEL (priv->label_loan_capital_devise), priv->code_devise);
		if (!s_loan->currency_number)
			s_loan->currency_number = devise;

		/* capital emprunte */
		real = gsb_real_double_to_real (s_loan->capital);
		if (real.mantissa > 0)
		{
			tmp_str = utils_real_get_string_with_currency (real, devise, FALSE);
			gtk_entry_set_text (GTK_ENTRY (priv->entry_loan_capital), tmp_str);
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_loan_capital), GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
			g_free (tmp_str);
		}
		else
		{
			gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_loan_capital), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
		}

		/* duree du credit en mois */
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_loan_duration), (gdouble) s_loan->duree);

		/* Annuel rate interest */
		gtk_spin_button_set_digits (GTK_SPIN_BUTTON (priv->spinbutton_rate_interest), BET_TAUX_DIGITS);
		gtk_adjustment_set_step_increment (GTK_ADJUSTMENT (priv->adjustment_rate_interest),
										   bet_data_finance_get_bet_taux_step (BET_TAUX_DIGITS));
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_rate_interest), s_loan->annual_rate);

		/* set type taux */
		if (s_loan->type_taux == 2)
		{
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_type_taux_3), TRUE);

			/* set percentage fees */
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (priv->spinbutton_percentage_fees), s_loan->percentage_fees);

			/* set fixed due amount */
			real = gsb_real_double_to_real (s_loan->fixed_due_amount);
			if (real.mantissa > 0)
			{
				tmp_str = utils_real_get_string_with_currency (real, devise, FALSE);
				gtk_entry_set_text (GTK_ENTRY (priv->entry_fixed_due_amount), tmp_str);
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_fixed_due_amount), GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
				g_free (tmp_str);
			}
			else
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_fixed_due_amount), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			}

			/* sensitive entry first fees */
			gtk_widget_set_sensitive (priv->entry_amount_first_fees, TRUE);

			/* unsensitive priv->checkbutton_invers_cols_cap_ech */
			gtk_widget_set_sensitive (priv->checkbutton_invers_cols_cap_ech, FALSE);
			s_loan->invers_cols_cap_ech = FALSE;

		}
		else
		{
			if (s_loan->type_taux == 1)
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_type_taux_2), TRUE);
			else
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->radiobutton_type_taux_1), TRUE);

			/* set fees */
			real = gsb_real_double_to_real (s_loan->amount_fees);
			if (real.mantissa > 0)
			{
				tmp_str = utils_real_get_string_with_currency (real, devise, FALSE);
				gtk_entry_set_text (GTK_ENTRY (priv->entry_fees_per_month), tmp_str);
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_fees_per_month),
												GTK_ENTRY_ICON_PRIMARY,
												pixbuf_OK);
				g_free (tmp_str);
			}
			else
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_fees_per_month),
												GTK_ENTRY_ICON_PRIMARY,
												pixbuf_NOK);
			}

			/* sensitive entry first fees */
			gtk_widget_set_sensitive (priv->entry_amount_first_fees, FALSE);

			/* sensitive priv->checkbutton_invers_cols_cap_ech and set */
			gtk_widget_set_sensitive (priv->checkbutton_invers_cols_cap_ech, TRUE);
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_invers_cols_cap_ech),
										  s_loan->invers_cols_cap_ech);
		}

		/* adapt display */
		widget_loan_adapt_display_from_type_taux (s_loan->type_taux, priv);

		/* set date of first repayment */
		gsb_calendar_entry_set_date (priv->entry_first_date, s_loan->first_date);

		/*  is first repayment is different */
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_amount_first_is_different),
									  s_loan->first_is_different);
		if (s_loan->first_is_different)
		{
			/* first capital emprunte */
			real = gsb_real_double_to_real (s_loan->first_capital);
			if (real.mantissa > 0)
			{
				tmp_str = utils_real_get_string_with_currency (real, devise, FALSE);
				gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_capital), tmp_str);
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_capital),
												GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
				g_free (tmp_str);
			}
			else
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_capital),
												GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			gtk_widget_set_sensitive (priv->entry_amount_first_capital, TRUE);

			/* first fees */
			real = gsb_real_double_to_real (s_loan->first_fees);
			if (real.mantissa > 0)
			{
				tmp_str = utils_real_get_string_with_currency (real, devise, FALSE);
				gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_fees), tmp_str);
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_fees), GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
				g_free (tmp_str);
			}
			else
			{
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_fees), GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);
			}

			gtk_widget_set_sensitive (priv->entry_amount_first_fees, TRUE);

			/* first interests */
			real = gsb_real_double_to_real (s_loan->first_interests);
			if (real.mantissa > 0)
			{
				tmp_str = utils_real_get_string_with_currency (real, devise, FALSE);
				gtk_entry_set_text (GTK_ENTRY (priv->entry_amount_first_interests), tmp_str);
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_interests),
												GTK_ENTRY_ICON_PRIMARY, pixbuf_OK);
				g_free (tmp_str);
			}
			else
				gtk_entry_set_icon_from_pixbuf (GTK_ENTRY (priv->entry_amount_first_interests),
												GTK_ENTRY_ICON_PRIMARY, pixbuf_NOK);

			gtk_widget_set_sensitive (priv->entry_amount_first_interests, TRUE);

		}

		/* set checkbutton_init_sch_with_loan */
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_init_sch_with_loan),
									  s_loan->init_sch_with_loan);

		/* set button_init_scheduled */
		if (s_loan->init_sch_with_loan)
			gtk_button_set_label (GTK_BUTTON (priv->button_init_scheduled), _("Delete scheduled transaction"));
		else
			gtk_button_set_label (GTK_BUTTON (priv->button_init_scheduled), _("Create new scheduled transaction"));

		gtk_widget_set_sensitive (priv->button_init_scheduled, s_loan->init_sch_with_loan);
	}

	/* Connect signal */
    /* callback for checkbutton amount_first_is_different */
    g_signal_connect (priv->checkbutton_amount_first_is_different,
					  "toggled",
					  G_CALLBACK (widget_loan_checkbutton_amount_first_toggled),
					  w_loan);

    /* callback for entry loan_capital */
    g_signal_connect (priv->entry_loan_capital,
					  "focus-in-event",
					  G_CALLBACK (widget_loan_entry_focus_in),
					  w_loan);
    g_signal_connect (priv->entry_loan_capital,
					  "focus-out-event",
					  G_CALLBACK (widget_loan_entry_focus_out),
					  w_loan);
	g_signal_connect (priv->entry_loan_capital,
					  "delete-text",
					  G_CALLBACK (widget_loan_entry_deleted),
					  w_loan);

    /* callback for calendar entry */
    g_signal_connect (priv->entry_first_date,
					  "focus-in-event",
					  G_CALLBACK (widget_loan_entry_focus_in),
					  w_loan);
    g_signal_connect (priv->entry_first_date,
					  "focus-out-event",
					  G_CALLBACK (widget_loan_entry_focus_out),
					  w_loan);

	/* callback for spinbutton percentage fees */
	g_signal_connect (priv->spinbutton_percentage_fees,
					  "focus-in-event",
					  G_CALLBACK (widget_loan_entry_focus_in),
					  w_loan);
	g_signal_connect (priv->spinbutton_percentage_fees,
					  "focus-out-event",
					  G_CALLBACK (widget_loan_entry_focus_out),
					  w_loan);

	/* callback for entry fixed_due_amount */
	g_signal_connect (priv->entry_fixed_due_amount,
					  "focus-in-event",
					  G_CALLBACK (widget_loan_entry_focus_in),
					  w_loan);
	g_signal_connect (priv->entry_fixed_due_amount,
					  "focus-out-event",
					  G_CALLBACK (widget_loan_entry_focus_out),
					  w_loan);
	g_signal_connect (priv->entry_fixed_due_amount,
					  "delete-text",
					  G_CALLBACK (widget_loan_entry_deleted),
					  w_loan);

	/* callback for entry fees_per_month */
	g_signal_connect (priv->entry_fees_per_month,
					  "focus-in-event",
					  G_CALLBACK (widget_loan_entry_focus_in),
					  w_loan);
	g_signal_connect (priv->entry_fees_per_month,
					  "focus-out-event",
					  G_CALLBACK (widget_loan_entry_focus_out),
					  w_loan);
	g_signal_connect (priv->entry_fees_per_month,
					  "delete-text",
					  G_CALLBACK (widget_loan_entry_deleted),
					  w_loan);

    /* callback for entry amount_first_capital */
    g_signal_connect (priv->entry_amount_first_capital,
					  "focus-in-event",
					  G_CALLBACK (widget_loan_entry_focus_in),
					  w_loan);
    g_signal_connect (priv->entry_amount_first_capital,
					  "focus-out-event",
					  G_CALLBACK (widget_loan_entry_focus_out),
					  w_loan);
	g_signal_connect (priv->entry_amount_first_capital,
					  "delete-text",
					  G_CALLBACK (widget_loan_entry_deleted),
					  w_loan);

    /* callback for entry amount_first_fees */
    g_signal_connect (priv->entry_amount_first_fees,
					  "focus-in-event",
					  G_CALLBACK (widget_loan_entry_focus_in),
					  w_loan);
    g_signal_connect (priv->entry_amount_first_fees,
					  "focus-out-event",
					  G_CALLBACK (widget_loan_entry_focus_out),
					  w_loan);
	g_signal_connect (priv->entry_amount_first_fees,
					  "delete-text",
					  G_CALLBACK (widget_loan_entry_deleted),
					  w_loan);

    /* callback for entry amount_first_interests */
    g_signal_connect (priv->entry_amount_first_interests,
					  "focus-in-event",
					  G_CALLBACK (widget_loan_entry_focus_in),
					  w_loan);
    g_signal_connect (priv->entry_amount_first_interests,
					  "focus-out-event",
					  G_CALLBACK (widget_loan_entry_focus_out),
					  w_loan);
	g_signal_connect (priv->entry_amount_first_interests,
					  "delete-text",
					  G_CALLBACK (widget_loan_entry_deleted),
					  w_loan);

	/* callback for spinbutton loan_duration */
    g_signal_connect (priv->spinbutton_loan_duration,
					  "focus-in-event",
					  G_CALLBACK (widget_loan_entry_focus_in),
					  w_loan);
    g_signal_connect (priv->spinbutton_loan_duration,
					  "focus-out-event",
					  G_CALLBACK (widget_loan_entry_focus_out),
					  w_loan);

	/* callback for spinbutton rate_interest */
    g_signal_connect (priv->spinbutton_rate_interest,
					  "focus-in-event",
					  G_CALLBACK (widget_loan_entry_focus_in),
					  w_loan);
    g_signal_connect (priv->spinbutton_rate_interest,
					  "focus-out-event",
					  G_CALLBACK (widget_loan_entry_focus_out),
					  w_loan);

	/* callback for radiobutton type_taux_1 */
	g_signal_connect (G_OBJECT (priv->radiobutton_type_taux_1),
					  "toggled",
					  G_CALLBACK (widget_loan_radiobutton_type_taux_toggled),
					  w_loan);

	/* callback for radiobutton type_taux_2 */
	g_signal_connect (G_OBJECT (priv->radiobutton_type_taux_2),
					  "toggled",
					  G_CALLBACK (widget_loan_radiobutton_type_taux_toggled),
					  w_loan);

	/* callback for radiobutton type_taux_3 */
	g_signal_connect (G_OBJECT (priv->radiobutton_type_taux_3),
					  "toggled",
					  G_CALLBACK (widget_loan_radiobutton_type_taux_toggled),
					  w_loan);

	/* callback for checkbutton_init_sch_with_loan */
    g_signal_connect (priv->checkbutton_init_sch_with_loan,
					  "toggled",
					  G_CALLBACK (widget_loan_checkbutton_init_sch_with_loan_toggle),
					  w_loan);

	/* callback for button_init_scheduled */
	if (s_loan && s_loan->associated_account)
		g_signal_connect (priv->button_init_scheduled,
						  "clicked",
						  G_CALLBACK (widget_loan_button_delete_scheduled_clicked),
						  w_loan);
	else
		g_signal_connect (priv->button_init_scheduled,
						  "clicked",
						  G_CALLBACK (widget_loan_button_init_scheduled_clicked),
						  w_loan);

    /* callback for checkbutton_invers_cols_cap_ech */
    g_signal_connect (priv->checkbutton_invers_cols_cap_ech,
					  "toggled",
					  G_CALLBACK (widget_loan_checkbutton_invers_cols_cap_ech),
					  w_loan);

	/* callback for button_amortization_table */
    g_signal_connect (priv->button_amortization_table,
					  "clicked",
					  G_CALLBACK (widget_loan_button_amortization_table_clicked),
					  w_loan);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_loan_init (WidgetLoan *w_loan)
{

	gtk_widget_init_template (GTK_WIDGET (w_loan));

	/* set icons */
	pixbuf_OK = gdk_pixbuf_new_from_resource ("/org/gtk/grisbi/images/gtk-apply-16.png", NULL);
	pixbuf_NOK = gdk_pixbuf_new_from_resource ("/org/gtk/grisbi/images/gtk-warning-16.png", NULL);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_loan_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_loan_parent_class)->dispose (object);
}

static void widget_loan_class_init (WidgetLoanClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_loan_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_loan.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, vbox_loan_data);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, grid_loan_data);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, adjustment_rate_interest);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, button_amortization_table);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, button_init_scheduled);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, checkbutton_amount_first_is_different);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, checkbutton_init_sch_with_loan);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, checkbutton_invers_cols_cap_ech);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, entry_amount_first_capital);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, entry_amount_first_fees);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, entry_amount_first_interests);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, entry_fees_per_month);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, entry_first_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, entry_fixed_due_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, entry_loan_capital);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, label_fees_per_month_devise);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, label_first_capital_devise);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, label_first_fees_devise);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, label_first_interests_devise);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, label_fixed_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, label_fixed_amount_devise);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, label_loan_capital_devise);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, radiobutton_type_taux_1);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, radiobutton_type_taux_2);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, radiobutton_type_taux_3);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, spinbutton_percentage_fees);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, spinbutton_loan_duration);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, spinbutton_rate_interest);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetLoan, vbox_fees_per_month);
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
WidgetLoan *widget_loan_new (LoanStruct *s_loan)
{
	WidgetLoan *w_loan;

	devel_debug (NULL);

	w_loan = g_object_new (WIDGET_LOAN_TYPE, NULL);
	widget_loan_setup_widget (w_loan, s_loan);

	return w_loan;
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

