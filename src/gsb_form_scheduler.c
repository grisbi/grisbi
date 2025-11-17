/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                 2009-2016 Pierre Biava (grisbi@pierre.biava.name)          */
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
 * \file gsb_form_scheduler.c
 * work with the form of the scheduler
 */


#include "config.h"

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_form_scheduler.h"
#include "dialog.h"
#include "grisbi_win.h"
#include "gsb_account.h"
#include "gsb_calendar_entry.h"
#include "gsb_combo_box.h"
#include "gsb_currency.h"
#include "gsb_data_form.h"
#include "gsb_data_payment.h"
#include "gsb_data_scheduled.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_fyear.h"
#include "gsb_payment_method.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "gtk_combofix.h"
#include "structures.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
/** contains the list of the scheduled elements, ie list of link
 * between an element number and the pointer of its widget
 * for now, this list if filled at the opening of grisbi and never erased */
static GSList *scheduled_element_list = NULL;

/**
 * used when change the account with the button, have only the new account,
 * so can use that variable which will be set to the new account at the end
 * of the callback when change the button (gsb_form_scheduler_change_account)
 * */
static gint last_account_number = 0;
/*END_STATIC*/

/**
 * \struct
 * associate an element number and a string value
 * used to keed the content of the form when changing the account
 * because the form is redrawed */
typedef struct _ContentElement			ContentElement;


struct _ContentElement{
	gint element_number;
	gchar *element_string;
	gint element_int;
};

/******************************************************************************/
/* Private Methods                                                            */
/******************************************************************************/
/**
 * free a content list created by gsb_form_scheduler_get_content_list
 *
 * \param content_list
 *
 * \return
 **/
static void gsb_form_scheduler_free_content_list (GSList *content_list)
{
	GSList *list_tmp;

	list_tmp = content_list;

	while (list_tmp)
	{
		ContentElement *element;

		element = list_tmp->data;

		/* normally, cannot happen */
		if (!element)
			continue;

		if (element->element_string)
			g_free (element->element_string);
		g_free (element);

		list_tmp = list_tmp->next;
	}
	g_slist_free (content_list);
}

/**
 * called when the frequency button is changed
 * show/hide the necessary widget according to its state
 *
 * \param combo_box
 *
 * \return FALSE
 **/
static gboolean gsb_form_scheduler_frequency_button_changed (GtkWidget *combo_box,
															 gpointer null)
{
	gchar *selected_item;

	selected_item = gsb_combo_box_get_active_text (GTK_COMBO_BOX (combo_box));

	if (!strcmp (selected_item, _("Once")))
	{
		gtk_widget_hide (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_LIMIT_DATE));
		gtk_widget_hide (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_ENTRY));
		gtk_widget_hide (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_BUTTON));
	}
	else
	{
		gtk_widget_show (gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_LIMIT_DATE));
		if (!strcmp (selected_item, _("Custom")))
		{
			gtk_widget_show (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_ENTRY));
			gtk_widget_show (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_BUTTON));
		}
		else
		{
			gtk_widget_hide (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_ENTRY));
			gtk_widget_hide (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_BUTTON));
		}
	}
	g_free (selected_item);

	return FALSE;
}

/**
 * get the automatic/manual mode from the form's button
 *
 * \param
 *
 * \return TRUE if automatic, FALSE if manual, -1 if nothing selected (normaly should not append)
 **/
static gint gsb_form_scheduler_get_auto (void)
{
	GtkWidget *button;

	button = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_AUTO);

	/* if no automatic button, go away... */
	if (!button)
		return FALSE;

	return gsb_combo_box_get_index (button);
}

/**
 * get the content in the form, and set it in a list composed
 * of each element with their content (list of struct ContentElement
 *
 * \param
 *
 * \return a newly allocated list
 **/
static GSList *gsb_form_scheduler_get_content_list (void)
{
	GSList *content_list = NULL;
	GSList *tmp_list;

	tmp_list = gsb_form_widget_get_list ();
	while (tmp_list)
	{
		FormElement *element;

		element = tmp_list->data;

		if (gtk_widget_get_visible (element->element_widget))
		{
			ContentElement *element_save;

			switch (element->element_number)
			{
				/* first, check the entries */
				case TRANSACTION_FORM_DATE:
				case TRANSACTION_FORM_VALUE_DATE:
				case TRANSACTION_FORM_DEBIT:
				case TRANSACTION_FORM_CREDIT:
				case TRANSACTION_FORM_NOTES:
				case TRANSACTION_FORM_BANK:
				case TRANSACTION_FORM_VOUCHER:
					if (!gsb_form_widget_check_empty (element->element_widget))
					{
					element_save = g_malloc0 (sizeof (ContentElement));
					element_save->element_number = element->element_number;
					element_save->element_string = my_strdup (gtk_entry_get_text
															  (GTK_ENTRY (element->element_widget)));
					content_list = g_slist_append (content_list, element_save);
					}
					break;

				case TRANSACTION_FORM_TYPE:
					element_save = g_malloc0 (sizeof (ContentElement));
					element_save->element_number = element->element_number;
					element_save->element_int = gsb_payment_method_get_selected_number (element->element_widget);
					content_list = g_slist_append (content_list, element_save);
					break;

				case TRANSACTION_FORM_CONTRA:
					element_save = g_malloc0 (sizeof (ContentElement));
					element_save->element_number = element->element_number;
					element_save->element_int = gsb_payment_method_get_selected_number (element->element_widget);
					content_list = g_slist_append (content_list, element_save);
					break;

				case TRANSACTION_FORM_EXERCICE:
					element_save = g_malloc0 (sizeof (ContentElement));
					element_save->element_number = element->element_number;
					element_save->element_int = gsb_fyear_get_fyear_from_combobox (element->element_widget,
																				   NULL);
					content_list = g_slist_append (content_list, element_save);
					break;

				case TRANSACTION_FORM_DEVISE:
					element_save = g_malloc0 (sizeof (ContentElement));
					element_save->element_number = element->element_number;
					element_save->element_int = gsb_currency_get_currency_from_combobox (element->element_widget);
					content_list = g_slist_append (content_list, element_save);
					break;

					/* check the combofix */
				case TRANSACTION_FORM_PARTY:
				case TRANSACTION_FORM_CATEGORY:
				case TRANSACTION_FORM_BUDGET:
					if (!gsb_form_widget_check_empty (element->element_widget))
					{
						const gchar *tmp_str;

						tmp_str = gtk_combofix_get_text (GTK_COMBOFIX (element->element_widget));
						element_save = g_malloc0 (sizeof (ContentElement));
						element_save->element_number = element->element_number;
						element_save->element_string = my_strdup (tmp_str);
						content_list = g_slist_append (content_list, element_save);
					}
					break;
			}
		}
		tmp_list = tmp_list->next;
	}

	return content_list;
}

/**
 * get the frequency from the form's button
 *
 * \param
 *
 * \return SCHEDULER_PERIODICITY_x_VIEW (see gsb_scheduler_list.h), -1 if nothing selected (normaly should not append)
 **/
static gint gsb_form_scheduler_get_frequency (void)
{
	GtkWidget *button;

	button = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_BUTTON);

	/* if no automatic button, go away... */
	if (!button)
		return FALSE;

	return gsb_combo_box_get_index (button);
}

/**
 * get the user frequency from the form's scheduled part
 *
 * \param
 *
 * \return a gint contains the user frequency
 **/
static gint gsb_form_scheduler_get_frequency_user (void)
{
	GtkWidget *entry;

	entry = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_ENTRY);

	/* if no entry, go away... */
	if (!entry || gsb_form_widget_check_empty (entry))
		return 0;

	return utils_str_atoi (gtk_entry_get_text (GTK_ENTRY (entry)));
}

/**
 * get the user frequency from the form's button
 *
 * \param
 *
 * \return 0 : Days, 1 : Months, 2 : Years, -1 if nothing selected (normaly should not append)
 **/
static gint gsb_form_scheduler_get_frequency_user_button (void)
{
	GtkWidget *button;

	button = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_BUTTON);

	/* if no automatic button, go away... */
	if (!button)
		return FALSE;

	return gsb_combo_box_get_index (button);
}

/**
 * get the limit date from the form's scheduled part
 *
 * \param
 *
 * \return a GDate or NULL if nothing into the entry or problem
 **/
static GDate *gsb_form_scheduler_get_limit_date (void)
{
	GtkWidget *entry;
	GDate *date;

	entry = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_LIMIT_DATE);

	/* if no entry, go away... */
	if (!entry || gsb_form_widget_check_empty (entry) || !gtk_widget_get_visible (entry))
		return NULL;

	date = gsb_calendar_entry_get_date (entry);

	return date;
}

/**
 * set the account in the account button
 * this will automatickly change the content of the form
 *
 * \param account_number
 *
 * \return FALSE if not done, TRUE if ok
 **/
static gboolean gsb_form_scheduler_set_account (gint account_number)
{
	GtkTreeIter iter;
	GtkWidget *button;
	GtkTreeModel *model;

	button = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT);

	/* if no account button, go away... */
	if (!button)
		return FALSE;

	model = gtk_combo_box_get_model (GTK_COMBO_BOX (button));
	if (gtk_tree_model_get_iter_first (model, &iter))
	{
		do
		{
			gint number;

			gtk_tree_model_get (model, &iter, 1, &number, -1);
			if (number == account_number)
			{
				gtk_combo_box_set_active_iter (GTK_COMBO_BOX (button), &iter);

				return TRUE;
			}
		}
		while (gtk_tree_model_iter_next (model, &iter));
	}

	return FALSE;
}

/**
 * set the automatic/manual mode on the form's button
 *
 * \param automatic TRUE to set automatic, FALSE to set manual
 *
 * \return TRUE if ok, FALSE else
 **/
static gboolean gsb_form_scheduler_set_auto (gboolean automatic)
{
	GtkWidget *button;

	button = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_AUTO);

	/* if no automatic button, go away... */
	if (!button)
		return FALSE;

	return gsb_combo_box_set_index (button, automatic);
}

/**
 * set the content in the form according to the content of the list
 * of struct ContentElement given in param
 *
 * \param content_list the list of struct ContentElement
 *
 * \return
 **/
static void gsb_form_scheduler_set_content_list (GSList *content_list)
{
	GSList *list_tmp;
	gint account_number;

	account_number = gsb_form_scheduler_get_account ();

	/* see each saved element */
	list_tmp = content_list;
	while (list_tmp)
	{
		ContentElement *element;
		GSList *form_list;

		element = list_tmp->data;

		/* normally, cannot happen */
		if (!element)
			continue;

		/* we look for that element in the form */
		form_list = gsb_form_widget_get_list ();

		while (form_list)
		{
			FormElement *form_element;

			form_element = form_list->data;

			if (!gtk_widget_get_visible (form_element->element_widget)
			|| form_element->element_number != element->element_number)
			{
				form_list = form_list->next;
				continue;
			}

			switch (form_element->element_number)
			{
				/* first, the entries */
				case TRANSACTION_FORM_DATE:
				case TRANSACTION_FORM_VALUE_DATE:
				case TRANSACTION_FORM_DEBIT:
				case TRANSACTION_FORM_CREDIT:
				case TRANSACTION_FORM_NOTES:
				case TRANSACTION_FORM_BANK:
				case TRANSACTION_FORM_VOUCHER:
					gsb_form_entry_get_focus (form_element->element_widget);
					if (element->element_string)
						gtk_entry_set_text (GTK_ENTRY (form_element->element_widget), element->element_string);
					break;

				case TRANSACTION_FORM_TYPE:
					gsb_payment_method_create_combo_list (form_element->element_widget,
														  gsb_data_payment_get_sign (element->element_int),
														  account_number,
														  0,
														  FALSE);
					gsb_payment_method_set_combobox_history (form_element->element_widget,
															 gsb_data_payment_get_similar (element->element_int, account_number),
															 TRUE);
					break;

				case TRANSACTION_FORM_CONTRA:
					gsb_payment_method_create_combo_list (form_element->element_widget,
														  gsb_data_payment_get_sign (element->element_int),
														  account_number,
														  0,
														  TRUE);
					gsb_payment_method_set_combobox_history (form_element->element_widget,
															 gsb_data_payment_get_similar (element->element_int, account_number),
															 TRUE);
					break;

				case TRANSACTION_FORM_EXERCICE:
					gsb_fyear_set_combobox_history (form_element->element_widget, element->element_int);
					break;

				case TRANSACTION_FORM_DEVISE:
					gsb_currency_set_combobox_history (form_element->element_widget, element->element_int);
					break;

					/* check the combofix */
				case TRANSACTION_FORM_PARTY:
				case TRANSACTION_FORM_CATEGORY:
				case TRANSACTION_FORM_BUDGET:
					gsb_form_entry_get_focus (form_element->element_widget);
					if (element->element_string)
						gtk_combofix_set_text (GTK_COMBOFIX (form_element->element_widget), element->element_string);
					break;
				}
				form_list = form_list->next;
		}
	list_tmp = list_tmp->next;
	}
}

/**
 * set the user frequency in the form's scheduled part
 *
 * \param user_freq the user frequency number
 *
 * \return TRUE if ok, FALSE else
 **/
static gboolean gsb_form_scheduler_set_frequency_user (gint user_freq)
{
	GtkWidget *entry;
	gchar *string;

	entry = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_ENTRY);

	/* if no entry, go away... */
	if (!entry || !gtk_widget_get_visible (entry))
		return FALSE;

	string = utils_str_itoa (user_freq);
	if (!string)
		return FALSE;

	gsb_form_widget_set_empty (entry, FALSE);
	gtk_entry_set_text (GTK_ENTRY (entry), string);
	g_free (string) ;

	return TRUE;
}

/**
 * set the user frequency on the form's button
 *
 * \param frequency 0 : Days, 1 : Months, 2 : Years
 *
 * \return TRUE if ok, FALSE else
 **/
static gboolean gsb_form_scheduler_set_frequency_user_button (gboolean automatic)
{
	GtkWidget *button;

	button = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_BUTTON);

	/* if no automatic button, go away... */
	if (!button || !gtk_widget_get_visible (button))
		return FALSE;

	return gsb_combo_box_set_index (button, automatic);
}

/**
 * set the limit date in the form's scheduled part
 *
 * \param date a GDate or NULL if we want to free the entry
 *
 * \return TRUE if ok, FALSE else
 **/
static gboolean gsb_form_scheduler_set_limit_date (GDate *date)
{
	GtkWidget *entry;

	entry = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_LIMIT_DATE);

	/* if no entry, go away... */
	if (!entry || !gtk_widget_get_visible (entry))
		return FALSE;

	if (!date)
	{
		gsb_form_widget_set_empty (entry, TRUE);
		gtk_entry_set_placeholder_text (GTK_ENTRY (entry), _("Limit date"));
	}
	else
	{
		gsb_calendar_entry_set_date (entry, date);
		gsb_form_widget_set_empty (entry, FALSE);
	}

	return TRUE;
}

/******************************************************************************/
/* Public Methods                                                             */
/******************************************************************************/
/**
 * create the scheduled part : that widgets are created at the beginning
 * and normally never destroyed, they are showed only for
 * scheduled transactions
 * Cela ne fonctionne pas : tous les widgets sont détruits par la
 * fonction gsb_form_create_widgets ()
 *
 * \param table a GtkTable with the dimension SCHEDULED_HEIGHT*SCHEDULED_WIDTH to be filled
 *
 * \return FALSE
 **/
gboolean gsb_form_scheduler_create (GtkWidget *table)
{
	gint row, column;
	FormElement *element;
	GrisbiWinEtat *w_etat;

	devel_debug (NULL);
	if (!table)
		return FALSE;

	w_etat = grisbi_win_get_w_etat ();

	/* just in case... be sure that not created */
	if (scheduled_element_list)
		gsb_form_scheduler_free_list ();

	/* ok, now fill the form
	 * we play with height and width, but for now it's fix : 6 columns and 1 line */
	for (row=0 ; row < SCHEDULED_HEIGHT ; row++)
	for (column=0 ; column < SCHEDULED_WIDTH ; column++)
	{
		gint element_number;
		GtkWidget *widget = NULL;
		const gchar *tooltip_text = NULL;
		const gchar *text_auto [] = {N_("Manual"), N_("Automatic"), NULL};
		const gchar *text_frequency [] = {N_("Once"), N_("Weekly"), N_("Monthly"), N_("Bimonthly"),
						N_("Quarterly"), N_("Yearly"), N_("Custom"), NULL};
		const gchar *text_frequency_user [] = {N_("Days"), N_("Weeks"), N_("Months"), N_("Years"), NULL};

		element_number = row*SCHEDULED_WIDTH + column;
		switch (element_number)
		{
			case SCHEDULED_FORM_ACCOUNT:
				widget = gsb_account_create_combo_list (G_CALLBACK (gsb_form_scheduler_change_account), NULL, FALSE);
				gtk_widget_set_hexpand (widget, TRUE);
				if (w_etat->scheduler_set_default_account)
				{
					g_signal_handlers_block_by_func (widget, gsb_form_scheduler_change_account, NULL);
					gsb_account_set_combo_account_number (widget, w_etat->scheduler_default_account_number);
					g_signal_handlers_unblock_by_func (widget, gsb_form_scheduler_change_account, NULL);
				}
				else
					gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 0);

				tooltip_text = _("Choose the account");
				break;

			case SCHEDULED_FORM_AUTO:
				widget = gsb_combo_box_new_with_index (text_auto, NULL, NULL);
				gtk_widget_set_hexpand (widget, TRUE);
				tooltip_text = _("Automatic/manual scheduled transaction");
				break;

			case SCHEDULED_FORM_FREQUENCY_BUTTON:
				widget = gsb_combo_box_new_with_index (text_frequency,
													   G_CALLBACK (gsb_form_scheduler_frequency_button_changed),
													   NULL);
				gtk_widget_set_hexpand (widget, TRUE);
				tooltip_text = _("Frequency");
				break;

			case SCHEDULED_FORM_LIMIT_DATE:
				widget = gsb_calendar_entry_new (FALSE);
				gtk_widget_set_hexpand (widget, TRUE);
				g_signal_connect (G_OBJECT (widget),
								  "button-press-event",
								  G_CALLBACK (gsb_form_scheduler_button_press_event),
								  GINT_TO_POINTER (element_number));
				g_signal_connect (G_OBJECT (widget),
								  "focus-in-event",
								  G_CALLBACK (gsb_form_entry_get_focus),
								  GINT_TO_POINTER (element_number));
				g_signal_connect_after (G_OBJECT (widget),
										"focus-out-event",
										G_CALLBACK (gsb_form_scheduler_entry_lose_focus),
										GINT_TO_POINTER (element_number));
				tooltip_text = _("Limit date");
				break;

			case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
				widget = gtk_entry_new ();
				g_signal_connect (G_OBJECT (widget),
								  "focus-in-event",
								  G_CALLBACK (gsb_form_entry_get_focus),
								  GINT_TO_POINTER (element_number));
				g_signal_connect_after (G_OBJECT (widget),
										"focus-out-event",
										G_CALLBACK (gsb_form_scheduler_entry_lose_focus),
										GINT_TO_POINTER (element_number));
				tooltip_text = _("Own frequency");
				break;

			case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
				widget = gsb_combo_box_new_with_index (text_frequency_user, NULL, NULL);
				tooltip_text = _("Custom frequency");
				break;
		}

		if (!widget)
			continue;

		if (tooltip_text)
			gtk_widget_set_tooltip_text (GTK_WIDGET (widget), tooltip_text);

		/* save the element */
		element = g_malloc0 (sizeof (FormElement));
		element->element_number = element_number;
		element->element_widget = widget;
		scheduled_element_list = g_slist_append (scheduled_element_list, element);

		/* set in the form */
		gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
	}
	gsb_form_scheduler_clean ();

	return FALSE;
}

/**
 * destroy all the elements in scheduled_element_list
 * and free the list
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_form_scheduler_free_list (void)
{
	GSList *list_tmp;

	devel_debug (NULL);

	if (!scheduled_element_list)
		return FALSE;

	list_tmp = scheduled_element_list;
	while (list_tmp)
	{
		FormElement *element;

		element = list_tmp->data;
		g_free (element);

		list_tmp = list_tmp->next;
	}
	g_slist_free (scheduled_element_list);
	scheduled_element_list =NULL;

	return FALSE;
}

/**
 * callback called when changing the account from the form's button
 * re-fill the form but keep the values
 *
 * \param button
 * \param null
 *
 * \return FALSE
 **/
gboolean gsb_form_scheduler_change_account (GtkWidget *button,
											gpointer null)
{
	gint save_transaction;
	gint save_execute;
	GSList *content_list;
	gboolean is_split = FALSE;
	GtkWidget *category_entry;
	const gchar *tmp_str;
	gint new_account_number;

	devel_debug (NULL);
	new_account_number = gsb_form_get_account_number ();

	/* need to check first if split (see later) */
	category_entry = gsb_form_widget_get_widget (TRANSACTION_FORM_CATEGORY);
	if (category_entry)
	{
		tmp_str = gtk_combofix_get_text (GTK_COMBOFIX (category_entry));
		if (gsb_form_widget_check_empty (category_entry)
		 && tmp_str
		 && strlen (tmp_str) > 0
		 && !strcmp (tmp_str, _("Split of transaction")))
			/* ok it's a split */
			is_split = TRUE;
	}

	/* problem here : when change account, the form can be changed, with new or less widgets
	 * so we fill again de form
	 * but il the user fill the form and want to change after the account, it's annoying because
	 * filling again the form will lose all the data
	 * so first save the data and after filling the form, set back the data
	 * may still a problem : if for example we set a note, go to an account without notes, and
	 * go back to an account with a note, the first content of the note will be lost but it should
	 * be very rare to do that and i think very difficult to code something to keep that... */
	save_transaction = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (gsb_form_get_form_widget ()),
														   "transaction_number_in_form"));
	save_execute = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (gsb_form_get_form_widget ()), "execute_scheduled"));
	content_list = gsb_form_scheduler_get_content_list ();
	gsb_form_clean (new_account_number);

	/* a problem now, fill_from_account will clean the form,
	 * and make unsensitive some part of the form (method of payment...)
	 * and make sensitive some other part which could be unsensitive (for split for example)
	 * so we call gsb_form_set_sensitive, but 2 args, split or child.
	 * cannot be a child because child cannot access to the account button, so just to check
	 * if it's a split (done before)
	 */
	gsb_form_change_sensitive_buttons (TRUE);
	gsb_form_set_sensitive (is_split, FALSE);

	gsb_form_scheduler_set_content_list (content_list);
	gsb_form_scheduler_free_content_list (content_list);

	g_object_set_data (G_OBJECT (gsb_form_get_form_widget ()), "transaction_number_in_form", GINT_TO_POINTER (save_transaction));
	g_object_set_data (G_OBJECT (gsb_form_get_form_widget ()), "execute_scheduled",  GINT_TO_POINTER (save_execute));

	last_account_number = new_account_number;
	return FALSE;
}

/**
 * clean the scheduled form part
 * and set the default values
 *
 * \param account number
 *
 * \return FALSE
 **/
gboolean gsb_form_scheduler_clean (void)
{
	gint column;
	GtkWidget *widget;
	GrisbiWinEtat *w_etat;

	devel_debug (NULL);
	w_etat = grisbi_win_get_w_etat ();

	/* pbiava the execute flag is a gint : set to 0 */
	g_object_set_data (G_OBJECT (gsb_form_get_form_widget ()), "execute_scheduled", 0);

	/* first we show it, becaus hidden when execute a scheduled transaction */
	if (gsb_form_get_origin () == ORIGIN_VALUE_SCHEDULED)
	gtk_widget_show (gsb_form_get_scheduler_part ());

	/* clean the scheduled widget */
	for (column = 0 ; column < SCHEDULED_FORM_MAX_WIDGETS ; column++)
	{
		widget = gsb_form_scheduler_get_element_widget (column);

		/* some widgets can be set unsensitive because of the children of splits,
		 * so resensitive all to be sure */
		if (widget)
		{
			switch (column)
			{
				case SCHEDULED_FORM_ACCOUNT:
					if (w_etat->scheduler_set_default_account)
					{
						g_signal_handlers_block_by_func (widget, gsb_form_scheduler_change_account, NULL);
						gsb_account_set_combo_account_number (widget, w_etat->scheduler_default_account_number);
						g_signal_handlers_unblock_by_func (widget, gsb_form_scheduler_change_account, NULL);
					}
					else
						gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 0);
					gtk_widget_set_sensitive (widget, FALSE);
					break;

				case SCHEDULED_FORM_AUTO:
				case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
					gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 0);
					gtk_widget_set_sensitive (widget, FALSE);
					break;

				case SCHEDULED_FORM_FREQUENCY_BUTTON:
					g_signal_handlers_block_by_func (widget, gsb_form_scheduler_frequency_button_changed, NULL);
					gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 0);
					gtk_widget_set_sensitive (widget, FALSE);
					g_signal_handlers_unblock_by_func (widget, gsb_form_scheduler_frequency_button_changed, NULL);
					break;

				case SCHEDULED_FORM_LIMIT_DATE:
					gsb_form_widget_set_empty (widget, TRUE);
					gtk_entry_set_placeholder_text (GTK_ENTRY (widget), _("Limit date"));
					gtk_widget_set_sensitive (widget, TRUE);
					break;

				case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
					gsb_form_widget_set_empty (widget, TRUE);
					gtk_entry_set_placeholder_text (GTK_ENTRY (widget), _("Own frequency"));
					gtk_widget_set_sensitive (widget, TRUE);
					break;
			}
		}
	}

	return FALSE;
}

/**
 * sensitive the buttons of the scheduler part of the form
 *
 * \param sensitive TRUE or FALSE
 *
 * \return FALSE
 **/
gboolean gsb_form_scheduler_sensitive_buttons (gboolean sensitive)
{
	gint column;

	devel_debug_int (sensitive);

	/* clean the scheduled widget */
	for (column = 0 ; column < SCHEDULED_FORM_MAX_WIDGETS ; column++)
	{
		GtkWidget *widget;

		widget = gsb_form_scheduler_get_element_widget (column);
		if (widget)
		{
			switch (column)
			{
				case SCHEDULED_FORM_ACCOUNT:
				case SCHEDULED_FORM_AUTO:
				case SCHEDULED_FORM_FREQUENCY_BUTTON:
				case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
					gtk_widget_set_sensitive (widget, sensitive);
					break;
			}
		}
	}

	return FALSE;
}

/**
 * set the scheduler part of the form from the scheduled transaction given in param
 *
 * \param scheduled_number
 *
 * \return FALSE
 **/
gboolean gsb_form_scheduler_set (gint scheduled_number)
{
	devel_debug_int (scheduled_number);

	gsb_form_scheduler_sensitive_buttons (TRUE);

	/* if we are on a white split line, set all as the mother */
	if (scheduled_number < -1)
		scheduled_number = gsb_data_scheduled_get_mother_scheduled_number (scheduled_number);

    gsb_form_scheduler_set_account (gsb_data_scheduled_get_account_number (scheduled_number));
    gsb_form_scheduler_set_auto (gsb_data_scheduled_get_automatic_scheduled (scheduled_number));
    gsb_form_scheduler_set_frequency (gsb_data_scheduled_get_frequency (scheduled_number));
    gsb_form_scheduler_set_limit_date (gsb_data_scheduled_get_limit_date (scheduled_number));
    gsb_form_scheduler_set_frequency_user (gsb_data_scheduled_get_user_entry (scheduled_number));
    gsb_form_scheduler_set_frequency_user_button (gsb_data_scheduled_get_user_interval (scheduled_number));

	return FALSE;
}

/**
 * set the scheduler part of the form from the mother scheduled given in param
 *
 * \param scheduled_number
 * \param mother_number
 *
 * \return
 **/
void gsb_form_scheduler_set_from_mother (gint scheduled_number,
										 gint mother_number)
{
	gsb_data_scheduled_set_account_number (scheduled_number,
										   gsb_data_scheduled_get_account_number (mother_number));
	gsb_data_scheduled_set_automatic_scheduled (scheduled_number,
												gsb_data_scheduled_get_automatic_scheduled (mother_number));
	gsb_data_scheduled_set_frequency (scheduled_number,
									  gsb_data_scheduled_get_frequency (mother_number));
	gsb_data_scheduled_set_limit_date (scheduled_number,
									   gsb_data_scheduled_get_limit_date (mother_number));
	gsb_data_scheduled_set_user_entry (scheduled_number,
									   gsb_data_scheduled_get_user_entry (mother_number));
	gsb_data_scheduled_set_user_interval (scheduled_number,
										  gsb_data_scheduled_get_user_interval (mother_number));
}

/**
 * fill the scheduled transaction given in param with the content of
 * the scheduled part of the form (ie frequency...)
 *
 * \param scheduled_number
 *
 * \return FALSE
 **/
gboolean gsb_form_scheduler_get_scheduler_part (gint scheduled_number)
{
	if (!scheduled_number)
		return FALSE;

	/* needn't to fill the account number because set while creating the scheduled transaction */
	gsb_data_scheduled_set_account_number (scheduled_number, gsb_form_scheduler_get_account ());
	gsb_data_scheduled_set_automatic_scheduled (scheduled_number, gsb_form_scheduler_get_auto ());
	gsb_data_scheduled_set_frequency (scheduled_number, gsb_form_scheduler_get_frequency ());
	gsb_data_scheduled_set_limit_date (scheduled_number, gsb_form_scheduler_get_limit_date ());
	gsb_data_scheduled_set_user_interval (scheduled_number, gsb_form_scheduler_get_frequency_user_button ());
	gsb_data_scheduled_set_user_entry (scheduled_number, gsb_form_scheduler_get_frequency_user ());

	return FALSE;
}

/**
 * return the widget of the element_number given in param,
 * for the scheduler part of the form
 *
 * \param element_number
 *
 * \return a GtkWidget * or NULL
 **/
GtkWidget *gsb_form_scheduler_get_element_widget (gint element_number)
{
	return gsb_form_get_element_widget_from_list (element_number, scheduled_element_list);
}

/**
 * called when we press the button in an entry field in
 * the form for the scheduled part
 *
 * \param entry which receive the signal
 * \param ev can be NULL
 * \param ptr_origin a pointer to int on the element_number
 *
 * \return FALSE
 **/
gboolean gsb_form_scheduler_button_press_event (GtkWidget *entry,
												GdkEventButton *ev,
												gint *ptr_origin)
{
	GtkWidget *date_entry;

	/* set the form sensitive */
	gsb_form_change_sensitive_buttons (TRUE);

	/* set the current date into the date entry except if there is already something into the value date */
	date_entry = gsb_form_widget_get_widget (TRANSACTION_FORM_DATE);
	if (gsb_form_widget_check_empty (date_entry))
	{
		gtk_entry_set_text (GTK_ENTRY (date_entry), gsb_date_today ());
		gsb_form_widget_set_empty (date_entry, FALSE);
	}

	return FALSE;
}

/**
 * called when an entry lose the focus in the scheduled part
 *
 * \param entry
 * \param ev
 * \param ptr_origin a pointer gint which is the number of the element
 *
 * \return FALSE
 **/
gboolean gsb_form_scheduler_entry_lose_focus (GtkWidget *entry,
											  GdkEventFocus *ev,
											  gint *ptr_origin)
{
	gint element_number;

	devel_debug (NULL);

	element_number = GPOINTER_TO_INT (ptr_origin);
	switch (element_number)
	{
		case  SCHEDULED_FORM_LIMIT_DATE:
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				gsb_form_widget_set_empty (entry, TRUE);
				gtk_entry_set_placeholder_text (GTK_ENTRY (entry), _("Limit date"));
			}
			else if (!gsb_date_check_entry (entry))
			{
					gchar *tmp_str;

					tmp_str = g_strdup_printf (_("Invalid date %s"), gtk_entry_get_text (GTK_ENTRY (entry)));
					dialogue_error (tmp_str);
					g_free(tmp_str);
					gtk_editable_delete_text (GTK_EDITABLE (entry), 0, -1);
					gtk_widget_grab_focus (entry);

					return (FALSE);
			}
			break;

		case  SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
				gtk_entry_set_placeholder_text (GTK_ENTRY (entry), _("Own frequency"));
			break;

		default :
			break;
	}

	return FALSE;
}

/**
 * get the account number from the scheduled button and return it
 *
 * \param
 *
 * \return the account number or -2 if problem
 **/
gint gsb_form_scheduler_get_account (void)
{
	GtkWidget *button;
	gint account_number;

	button = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT);

	/* if no account button, go away... */
	if (!button)
		return -2;

	account_number = gsb_account_get_combo_account_number (button);
	if (account_number == -1)
		return -2;

	return account_number;
}

/**
 * set the frequency on the form's button
 *
 * \param frequency SCHEDULER_PERIODICITY_x_VIEW (see gsb_scheduler_list.h)
 *
 * \return TRUE if ok, FALSE else
 **/
gboolean gsb_form_scheduler_set_frequency (gint frequency)
{
	GtkWidget *button;

	button = gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_BUTTON);

	/* if no automatic button, go away... */
	if (!button)
		return FALSE;

	return gsb_combo_box_set_index (button, frequency);
}

/**
 * Clone the children of a splitted transaction to add the to the new splitted scheduled
 *
 * \param scheduled_transaction		the splitted scheduled transaction we want to add the children
 * \param transaction_number		the splitted transaction we want to clone the children
 *
 * \return FALSE
 **/
gboolean gsb_form_scheduler_recover_splits_of_transaction (gint scheduled_number,
														   gint transaction_number)
{
	GSList *tmp_list;

	/* first clone the transactions */
	gsb_transactions_list_splitted_to_scheduled (transaction_number, scheduled_number);

	/* add the children to the list */
	tmp_list = gsb_data_scheduled_get_scheduled_list ();
	while (tmp_list)
	{
		gint tmp_number;

		tmp_number = gsb_data_scheduled_get_scheduled_number (tmp_list->data);

		if (gsb_data_scheduled_get_mother_scheduled_number (tmp_number) == scheduled_number)
			gsb_scheduler_list_append_new_scheduled (tmp_number, NULL);

		tmp_list = tmp_list->next;
	}

	return FALSE;
}

/**
 * set fixed date if available for the scheduled transaction
 *
 * \param	sheduled number
 * \param	date
 *
 * \return
 **/
gint gsb_form_scheduler_get_last_day_of_month_dialog (gint scheduled_number,
													  GDate *date)
{
	GtkWidget *dialog = NULL;
	GtkWidget *button_keep;
	GtkWidget *button_force;
	gchar *msg;
	gchar *tmp_str;
	GDateDay day;
	gint fixed_date = 0;
	gint result;

	day = g_date_get_day (date);
	tmp_str = g_strdup_printf (_("Keep the %d"), day);
	msg = g_strdup_printf (_("Do you want to keep the \"%d\" as the date or to force the last day of the month?\n\n"), day);

	dialog = dialogue_special_no_run (GTK_MESSAGE_QUESTION,
									  GTK_BUTTONS_NONE,
									  msg,
									  _("This is the last day of the month"));

	button_keep = gtk_button_new_with_label (tmp_str);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_keep, GTK_RESPONSE_NO);

	button_force = gtk_button_new_with_label (_("Force the last day of the month"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_force, GTK_RESPONSE_YES);

	gtk_widget_set_can_default (button_keep, TRUE);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_NO);

	gtk_widget_show_all (dialog);

	result = gtk_dialog_run (GTK_DIALOG (dialog));

	if (result == GTK_RESPONSE_YES)
	{
		fixed_date = 31;
	}
	else
	{
		fixed_date = day;
	}

	g_free (tmp_str);
	g_free (msg);
	gtk_widget_destroy (dialog);

	return fixed_date;
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

