/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)	                      */
/*                      2009-2018 Pierre Biava (grisbi@pierre.biava.name)     */
/*          https://www.grisbi.org/                                            */
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
 * \file gsb_form_widget.c
 * convenient functions to work with the elements in the form
 */


#include "config.h"

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_form_widget.h"
#include "grisbi_app.h"
#include "gsb_calendar_entry.h"
#include "gsb_combo_box.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_form.h"
#include "gsb_data_payee.h"
#include "gsb_data_report.h"
#include "gsb_form.h"
#include "gsb_form_transaction.h"
#include "gsb_fyear.h"
#include "gsb_locale.h"
#include "gsb_payment_method.h"
#include "gtk_combofix.h"
#include "gsb_data_payment.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GSList *		form_list_widgets = NULL; /* contains a list of FormElement according to the current form */
static gchar *		old_debit = NULL;
static gchar *		old_credit = NULL;
static gint			old_credit_payment_number = 0;
static gint			old_debit_payment_number = 0;
static gchar *		old_credit_payment_content = NULL;
static gchar *		old_debit_payment_content = NULL;
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


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
static gboolean gsb_form_combo_selection_changed (GtkTreeSelection *tree_selection,
												  gint *ptr_origin)
{
    GtkWidget *widget;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *tmp_str;
    gint element_number;
    gint account_nb;

    element_number = GPOINTER_TO_INT (ptr_origin);

    if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (tree_selection), &model, &iter))
        return FALSE;

    gtk_tree_model_get (model, &iter, 1, &tmp_str, -1);

    if (tmp_str && strlen (tmp_str))
    {
        if (element_number == TRANSACTION_FORM_CATEGORY)
        {
			widget = gsb_form_widget_get_widget (TRANSACTION_FORM_DEVISE);
			if (widget != NULL && gtk_widget_get_visible (widget))
			{
				if (g_str_has_prefix (tmp_str, _("Transfer : ")))
				{
					account_nb = gsb_form_get_account_number ();
					if (g_str_has_suffix (tmp_str, gsb_data_account_get_name (account_nb)))
						gtk_widget_hide (gsb_form_widget_get_widget (TRANSACTION_FORM_CHANGE));
					else
						gtk_widget_show (gsb_form_widget_get_widget (TRANSACTION_FORM_CHANGE));
				}
				else
					gtk_widget_hide (gsb_form_widget_get_widget (TRANSACTION_FORM_CHANGE));
			}
			else
			{
				widget = gsb_form_widget_get_widget (TRANSACTION_FORM_CHANGE);
				if (widget != NULL)
					gtk_widget_hide (widget);
			}
        }
    }
    return FALSE;
}

/**
 * check if the given element can receive focus
 *
 * \param element_number
 *
 * \return TRUE : can receive focus, or FALSE
 **/
static gboolean gsb_form_widget_can_focus (gint element_number)
{
    GtkWidget *widget;

    /* if element_number is -1 or -2, the iteration while must
     * stop, so return TRUE */
    if (element_number == -1 || element_number == -2)
		return TRUE;

    widget = gsb_form_widget_get_widget (element_number);
	if (!widget)
		return FALSE;

	if (!gtk_widget_get_visible (widget))
		return FALSE;

	if (!gtk_widget_get_sensitive (widget))
		return FALSE;

    if (!(GTK_IS_COMBOFIX (widget)
		  || GTK_IS_ENTRY (widget)
		  || GTK_IS_BUTTON (widget)
		  || GTK_IS_COMBO_BOX (widget)))
	{
		return FALSE;
	}

	return TRUE;
}

/******************************************************************************/
/* Public Methods                                                             */
/******************************************************************************/
/**
 * return the list which contains the widgets of the form
 *
 * \param
 *
 * \return a GSList with the pointers to the widgets of the form
 **/
GSList *gsb_form_widget_get_list (void)
{
    return form_list_widgets;
}

/**
 * free the content of the list of the widget's form
 * so free the content of the form
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_form_widget_free_list (void)
{
	GtkWidget *entry;
	GSList *tmp_list;
    GtkWidget *widget_signals;

    devel_debug (NULL);

    if (!form_list_widgets)
        return FALSE;

    tmp_list = form_list_widgets;
    while (tmp_list)
    {
        FormElement *element;

        element = tmp_list->data;
        if (!element)
		{
			tmp_list = tmp_list->next;
            continue;
		}

        /* just to make sure... */
        if (element->element_widget)
        {
            if (GTK_IS_WIDGET (element->element_widget))
            {
                widget_signals = NULL;
                if (GTK_IS_ENTRY (element->element_widget))
                {
                    widget_signals = element->element_widget;
                }
                else if (GTK_IS_COMBOFIX (element->element_widget))
                {
                    widget_signals = gtk_combofix_get_entry (GTK_COMBOFIX (element->element_widget));
					/* if there is something in the combofix we destroy, the popup will
					 * be showed because destroying the gtk_entry will erase it directly,
					 * so the simpliest way to avoid that is to erase now the entry, but with
					 * gtk_combofix_set_text [cedric] (didn't succeed with another thing...) */
					gtk_combofix_set_text (GTK_COMBOFIX (element->element_widget), "");
                    gsb_form_widget_set_empty (GTK_WIDGET (element->element_widget), TRUE);
                }
				else if (GTK_IS_COMBO_BOX (element->element_widget))
				{
					/* Ajout du 24/07/2017 : pour une raison encore indéterminée, les éléments de type GtkComboBox */
					/* provoquent un warning de la part de gtk (version 3.22.11) lorsque le widget est détruit :                                 */
					/* (grisbi:10426): Gtk-CRITICAL **: gtk_widget_is_drawable: assertion 'GTK_IS_WIDGET (widget)' failed */
					g_signal_handlers_block_by_func (element->element_widget,
													 gsb_payment_method_changed_callback,
													 NULL);
					if (element->element_number == TRANSACTION_FORM_TYPE)
					{
						element->element_widget = NULL;
						g_free (element);
						tmp_list = tmp_list->next;
						continue;
					}
					else if (element->element_number == TRANSACTION_FORM_PARTY)
					{
						entry = gtk_combofix_get_entry (GTK_COMBOFIX (element->element_widget));
						widget_signals = entry;
					}
				}

                if (widget_signals)
                {
                    g_signal_handlers_disconnect_by_func (G_OBJECT (widget_signals),
														  G_CALLBACK (gsb_form_entry_get_focus),
														  GINT_TO_POINTER (element->element_number));
                    g_signal_handlers_disconnect_by_func (G_OBJECT (widget_signals),
                        								  G_CALLBACK (gsb_form_entry_lose_focus),
                        								  GINT_TO_POINTER (element->element_number));
                    g_signal_handlers_disconnect_by_func (G_OBJECT (widget_signals),
                        								  G_CALLBACK (gsb_form_button_press_event),
                        								  GINT_TO_POINTER (element->element_number));
                    g_signal_handlers_disconnect_by_func (G_OBJECT (widget_signals),
                        								  G_CALLBACK (gsb_form_key_press_event),
                        								  GINT_TO_POINTER (element->element_number));
					if (GTK_IS_COMBOFIX (widget_signals))
                    	g_signal_handlers_disconnect_by_func (G_OBJECT (widget_signals),
															  G_CALLBACK (gsb_form_combo_selection_changed),
                        									  GINT_TO_POINTER (element->element_number));
                }

				gtk_widget_destroy (element->element_widget);
                element->element_widget = NULL;
            }
        }
        else
        {
            alert_debug ("element_widget is NULL\n");
        }
        g_free (element);
        tmp_list = tmp_list->next;
    }
    g_slist_free (form_list_widgets);
    form_list_widgets = NULL;

	return FALSE;
}

/**
 * create and return the widget according to its element number
 *
 * \param element_number the number corresponding to the widget wanted
 * \param account_number used for method of payment and currency
 *
 * \return the widget or NULL (if the element number is zero)
 **/
GtkWidget *gsb_form_widget_create (gint element_number,
								   gint account_number)
{
    GtkWidget *widget;
    GSList *tmp_list;
	GrisbiWinEtat *w_etat;

	w_etat = grisbi_win_get_w_etat ();
	if (!element_number)
		return NULL;

    widget = NULL;
    switch (element_number)
    {
		case TRANSACTION_FORM_DATE:
		case TRANSACTION_FORM_VALUE_DATE:
			widget = gsb_calendar_entry_new (FALSE);
			break;

		case TRANSACTION_FORM_DEBIT:
			widget = gtk_entry_new ();
			g_object_set_data (G_OBJECT (widget), "element_number",
							   GINT_TO_POINTER (TRANSACTION_FORM_DEBIT));
			g_signal_connect (G_OBJECT (widget),
							  "changed",
							  G_CALLBACK (gsb_form_widget_amount_entry_changed),
							  NULL);
			break;
		case TRANSACTION_FORM_CREDIT:
			widget = gtk_entry_new ();
			g_object_set_data (G_OBJECT (widget), "element_number",
							   GINT_TO_POINTER (TRANSACTION_FORM_CREDIT));
			g_signal_connect (G_OBJECT (widget),
							  "changed",
							  G_CALLBACK (gsb_form_widget_amount_entry_changed),
							  NULL);
			break;

		case TRANSACTION_FORM_NOTES:
		case TRANSACTION_FORM_CHEQUE:
		case TRANSACTION_FORM_VOUCHER:
		case TRANSACTION_FORM_BANK:
			widget = gtk_entry_new();
			break;

		case TRANSACTION_FORM_EXERCICE:
			widget = gsb_fyear_make_combobox (TRUE);
			gtk_widget_set_tooltip_text (GTK_WIDGET (widget),
										 _("Choose the financial year"));
			break;

		case TRANSACTION_FORM_PARTY:
			tmp_list = gsb_data_payee_get_name_and_report_list ();
			widget = gtk_combofix_new_with_properties (tmp_list,
													   w_etat->combofix_force_payee,
													   w_etat->combofix_case_sensitive,
													   FALSE,
													   METATREE_PAYEE);
			gsb_data_payee_free_name_and_report_list (tmp_list);
			break;

		case TRANSACTION_FORM_CATEGORY:
        	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
			widget = gtk_combofix_new_with_properties (tmp_list,
													   w_etat->combofix_force_category,
													   w_etat->combofix_case_sensitive,
													   w_etat->combofix_mixed_sort,
													   METATREE_CATEGORY);
			gsb_data_categorie_free_name_list (tmp_list);
	    	break;

		case TRANSACTION_FORM_FREE:
			alert_debug ("TRANSACTION_FORM_FREE returns an empty widget");
			break;

		case TRANSACTION_FORM_BUDGET:
			tmp_list = gsb_data_budget_get_name_list (TRUE, TRUE);
			widget = gtk_combofix_new_with_properties (tmp_list,
													   w_etat->combofix_force_category,
													   w_etat->combofix_case_sensitive,
													   w_etat->combofix_mixed_sort,
													   METATREE_BUDGET);
			gsb_data_categorie_free_name_list (tmp_list);
			break;

		case TRANSACTION_FORM_TYPE:
			widget = gtk_combo_box_new ();
			gsb_payment_method_create_combo_list (widget,
												  GSB_PAYMENT_DEBIT,
												  account_number,
												  0,
												  FALSE);
			gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 0);
			gtk_widget_set_tooltip_text (GTK_WIDGET (widget),
										 _("Choose the method of payment"));
			g_signal_connect (G_OBJECT (widget),
							  "changed",
							  G_CALLBACK (gsb_payment_method_changed_callback),
							  NULL);
			break;

		case TRANSACTION_FORM_DEVISE:
			widget = gsb_currency_make_combobox (FALSE);
			gtk_widget_set_tooltip_text (GTK_WIDGET (widget), _("Choose currency"));
			gsb_currency_set_combobox_history (widget,
											   gsb_data_account_get_currency (account_number));
			g_signal_connect (G_OBJECT (widget),
							  "changed",
							  G_CALLBACK (gsb_form_transaction_currency_changed),
							  NULL);
			break;

		case TRANSACTION_FORM_CHANGE:
			widget = gtk_button_new_with_label (_("Change"));
			gtk_button_set_relief (GTK_BUTTON (widget), GTK_RELIEF_NONE);
			gtk_widget_set_tooltip_text (GTK_WIDGET (widget),
										 _("Define the change for that transaction"));
			g_signal_connect (G_OBJECT (widget),
							  "clicked",
							  G_CALLBACK (gsb_form_transaction_change_clicked),
							  NULL);
			break;

		case TRANSACTION_FORM_CONTRA:
			/* no menu at beginning, appened when choose the contra-account */
			widget = gtk_combo_box_new ();
			gtk_widget_set_tooltip_text (GTK_WIDGET (widget),
						  _("Contra-transaction method of payment"));
			break;

		case TRANSACTION_FORM_OP_NB:
		case TRANSACTION_FORM_MODE:
			widget = gtk_label_new (NULL);
			break;
    }

    if (widget)
    {
		/* force minimum width */
		gtk_widget_set_size_request (widget, FORM_COURT_WIDTH, -1);

		/* first, append the widget to the list */
		FormElement *element;

		element = g_malloc0 (sizeof (FormElement));
		element->element_number = element_number;
		element->element_widget = widget;
		form_list_widgets = g_slist_append (form_list_widgets, element);

		/* set the signals */
		if (GTK_IS_ENTRY (widget))
		{
			g_signal_connect (G_OBJECT (widget),
							  "focus-in-event",
							  G_CALLBACK (gsb_form_widget_entry_get_focus),
							  GINT_TO_POINTER (element_number));
			g_signal_connect (G_OBJECT (widget),
					   	  	  "focus-out-event",
					   	  	  G_CALLBACK (gsb_form_entry_lose_focus),
					   	  	  GINT_TO_POINTER (element_number));
			g_signal_connect (G_OBJECT (widget),
					   	  	  "button-press-event",
					   	  	  G_CALLBACK (gsb_form_button_press_event),
					   	  	  GINT_TO_POINTER (element_number));
			g_signal_connect (G_OBJECT (widget),
					   	  	  "key-press-event",
					   	  	  G_CALLBACK (gsb_form_key_press_event),
					   	  	  GINT_TO_POINTER (element_number));
		}
		else
		{
			if (GTK_IS_COMBOFIX (widget))
	    	{
				GtkWidget *entry = NULL;

				entry = gtk_combofix_get_entry (GTK_COMBOFIX (widget));
				if (GTK_IS_ENTRY (entry))
				{
					gtk_widget_set_size_request (widget, FORM_LONG_WIDTH, -1);

					g_signal_connect (G_OBJECT (entry),
									  "focus-in-event",
									  G_CALLBACK (gsb_form_entry_get_focus),
									  GINT_TO_POINTER (element_number));
					g_signal_connect (G_OBJECT (entry),
									  "focus-out-event",
									  G_CALLBACK (gsb_form_entry_lose_focus),
									  GINT_TO_POINTER (element_number));
					g_signal_connect (G_OBJECT (entry),
									  "button-press-event",
									  G_CALLBACK (gsb_form_button_press_event),
									  GINT_TO_POINTER (element_number));
					g_signal_connect (G_OBJECT (entry),
									  "key-press-event",
									  G_CALLBACK (gsb_form_key_press_event),
									  GINT_TO_POINTER (element_number));
				}
				gtk_combofix_set_selection_callback (GTK_COMBOFIX (widget),
													 G_CALLBACK (gsb_form_combo_selection_changed),
													 GINT_TO_POINTER (element_number));
			}
			else
				/* neither an entry, neither a combofix or combo_box*/
				g_signal_connect (G_OBJECT (widget),
								  "key-press-event",
								  G_CALLBACK (gsb_form_key_press_event),
								  GINT_TO_POINTER (element_number));
		}
    }
    else
	{
    	alert_debug ("Widget should not be NULL");
    }

    return widget;
}

/**
 * return the pointer to the widget corresponding to the given element
 *
 * \param element_number
 *
 * \return the widget or NULL
 **/
GtkWidget *gsb_form_widget_get_widget (gint element_number)
{
    return gsb_form_get_element_widget_from_list (element_number,
												  form_list_widgets);
}

/**
 * get an element number and return its name
 *
 * \param element_number
 *
 * \return the name of the element or NULL if problem
 **/
const gchar *gsb_form_widget_get_name (gint element_number)
{
    switch (element_number)
    {
		case -1:
			/* that value shouldn't be there, it shows that a gsb_data_form_... returns
			 * an error value */
			warning_debug ("gsb_form_widget_get_name : a value in the form is -1 "
						   "which should not happen.\nA gsb_data_form_... "
						   "function must have returned an error value...");
			return NULL;
			break;

		case TRANSACTION_FORM_DATE:
			return (N_("Date"));
			break;

		case TRANSACTION_FORM_DEBIT:
			return (N_("Debit"));
			break;

		case TRANSACTION_FORM_CREDIT:
			return (N_("Credit"));
			break;

		case TRANSACTION_FORM_VALUE_DATE:
			return (N_("Value date"));
			break;

		case TRANSACTION_FORM_EXERCICE:
			return (N_("Financial year"));
			break;

		case TRANSACTION_FORM_PARTY:
			return (N_("Payee"));
			break;

		case TRANSACTION_FORM_CATEGORY:
			return (N_("Categories : Sub-categories"));
			break;

		case TRANSACTION_FORM_FREE:
			return (N_("Free"));
			break;

		case TRANSACTION_FORM_BUDGET:
			return (N_("Budgetary line"));
			break;

		case TRANSACTION_FORM_NOTES:
			return (N_("Notes"));
			break;

		case TRANSACTION_FORM_TYPE:
			return (N_("Method of payment"));
			break;

		case TRANSACTION_FORM_CHEQUE:
			return (N_("Cheque/Transfer number"));
			break;

		case TRANSACTION_FORM_DEVISE:
			return (N_("Currency"));
			break;

		case TRANSACTION_FORM_CHANGE:
			return (N_("Change"));
			break;

		case TRANSACTION_FORM_VOUCHER:
			return (N_("Voucher"));
			break;

		case TRANSACTION_FORM_BANK:
			return (N_("Bank references"));
			break;

		case TRANSACTION_FORM_CONTRA:
			return (N_("Contra-transaction method of payment"));
			break;

		case TRANSACTION_FORM_OP_NB:
			return (N_("Transaction number"));
			break;

		case TRANSACTION_FORM_MODE:
			return (N_("Automatic/Manual"));
			break;
    }
    return NULL;
}

/**
 * return the number of the next valid element in the direction given in param
 *
 * \param account_number
 * \param element_number
 * \param direction GTK_DIR_LEFT, GTK_DIR_RIGHT, GTK_DIR_UP, GTK_DIR_DOWN
 *
 * \return 	the next element number
 * 		-1 if problem or not change
 * 		-2 if end of the form and need to finish it
 **/
gint gsb_form_widget_next_element (gint account_number,
								   gint element_number,
								   gint direction)
{
    gint row;
    gint column;
    gint return_value_number = 0;
    gint form_column_number;
    gint form_row_number;

    if (!gsb_data_form_look_for_value (element_number, &row, &column))
		return -1;

    form_column_number = gsb_data_form_get_nb_columns ();
    form_row_number = gsb_data_form_get_nb_rows ();

    while (!gsb_form_widget_can_focus (return_value_number))
    {
		switch (direction)
		{
			case GTK_DIR_LEFT:
			if (!column && !row)
			{
				/* we are at the upper left, go on the bottom right */
				column = form_column_number;
				row = form_row_number -1;
			}

			if (--column == -1)
			{
				column = form_column_number - 1;
				row--;
			}
			return_value_number = gsb_data_form_get_value (column, row);
			break;

			case GTK_DIR_RIGHT:
			if (column == (form_column_number - 1)
				 &&
				 row == (form_row_number - 1))
			{
				GrisbiAppConf *a_conf;

				a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

				/* we are on the bottom right, we finish the edition or
				 * go to the upper left */
				if (!a_conf->form_enter_key)
				{
				return_value_number = -2;
				continue;
				}
				column = -1;
				row = 0;
			}

			if (++column == form_column_number)
			{
				column = 0;
				row++;
			}
			return_value_number = gsb_data_form_get_value (column, row);
			break;

			case GTK_DIR_UP:
			if (!row)
			{
				return_value_number = -1;
				continue;
			}

			row--;
			return_value_number = gsb_data_form_get_value (column, row);
			break;

			case GTK_DIR_DOWN:
			if (row == (form_row_number - 1))
			{
				return_value_number = -1;
				continue;
			}
			row++;
			return_value_number = gsb_data_form_get_value (column, row);
			break;

			default:
			return_value_number = -1;
		}
    }
    return return_value_number;
}

/**
 * set the focus on the given element
 *
 * \param element_number
 *
 * \return
 **/
void gsb_form_widget_set_focus (gint element_number)
{
    GtkWidget *widget;

    widget = gsb_form_widget_get_widget (element_number);

	if (!widget)
		return;

	if (GTK_IS_COMBOFIX (widget))
	{
		GtkWidget *entry;

		entry = gtk_combofix_get_entry (GTK_COMBOFIX (widget));
		if (entry)
			gtk_widget_grab_focus (entry);
		else
			gtk_widget_grab_focus (widget);
	}
	else
		gtk_widget_grab_focus (widget);

	return;
}

/**
 * check if the entry or combofix given in param is empty (ie grey) or not
 *
 * \param entry must be an entry
 *
 * \return TRUE : entry is free, FALSE : not free
 **/
gboolean gsb_form_widget_check_empty (GtkWidget *entry)
{
    if (!entry)
		return FALSE;

	if (GTK_IS_ENTRY (entry))
		return GPOINTER_TO_INT (g_object_get_data (G_OBJECT(entry), "empty"));

	entry = gtk_combofix_get_entry (GTK_COMBOFIX (entry));
	if (GTK_IS_ENTRY (entry))
		return GPOINTER_TO_INT (g_object_get_data (G_OBJECT(entry), "empty"));

	return FALSE;
}

/**
 * set the style to the entry as empty or not
 *
 * \param entry must be an entry
 * \param empty TRUE or FALSE
 *
 * \return
 **/
void gsb_form_widget_set_empty (GtkWidget *entry,
                                gboolean empty)
{
    GtkStyleContext* context;

	if (!entry)
		return;

	if (!GTK_IS_ENTRY (entry))
		entry = gtk_combofix_get_entry (GTK_COMBOFIX (entry));

    if (!entry)
        return;

    context = gtk_widget_get_style_context (entry);

    if (!empty)
    {
        gtk_widget_set_name (entry, "form_entry");
        gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);
    }
    else
    {
        gtk_widget_set_name (entry, "form_entry_empty");
        gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);
    }

    g_object_set_data (G_OBJECT(entry), "empty", GINT_TO_POINTER (empty));
}

/**
 * called when an debit or credit entry get the focus,
 * if the entry is free, set it normal and erase the help content
 *
 * \param entry
 * \param ev
 *
 * \return FALSE
 **/
gboolean gsb_form_widget_entry_get_focus (GtkWidget *entry,
										  GdkEventFocus *ev,
										  gint *ptr_origin)
{
    GtkWidget *widget;
    GtkWidget *tmp_widget;
    gint element_number;
    gint account_number;

    /* still not found, if change the content of the form, something come in entry
     * which is nothing, so protect here */
	if (!GTK_IS_WIDGET (entry) || !GTK_IS_ENTRY (entry))
    	return FALSE;

    /* it clears the entry in question */
    if (gsb_form_widget_check_empty (entry))
    {
        gtk_entry_set_text (GTK_ENTRY (entry), "");
        gsb_form_widget_set_empty (entry, FALSE);
    }
    element_number = GPOINTER_TO_INT (ptr_origin);
    account_number = gsb_form_get_account_number ();

    switch (element_number)
    {
		case TRANSACTION_FORM_DEBIT :
			/* on met old_debit = NULl car avec g_free plantage */
			if (old_debit && strlen (old_debit) > 0)
				old_debit = NULL;

			/* save the credit if necessary */
			widget = gsb_form_widget_get_widget (TRANSACTION_FORM_CREDIT);
			if (!gsb_form_widget_check_empty (widget))
				old_credit = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));

			/* we change the payment method to adapt it for the debit */
			widget = gsb_form_widget_get_widget (TRANSACTION_FORM_TYPE);

			/* change the method of payment if necessary
			 * (if grey, it's a child of split so do nothing) */
			if (widget && gtk_widget_get_sensitive (widget))
			{
				/* change the signe of the method of payment and the contra */
				if (gsb_payment_method_get_combo_sign (widget) == GSB_PAYMENT_CREDIT)
				{
					old_credit_payment_number = gsb_payment_method_get_selected_number (widget);

					tmp_widget = gsb_form_widget_get_widget (TRANSACTION_FORM_CHEQUE);
					if (tmp_widget && gtk_widget_get_visible (tmp_widget))
					{
						if (old_credit_payment_content)
							g_free (old_credit_payment_content);
						if (gsb_form_widget_check_empty (tmp_widget) == FALSE)
							old_credit_payment_content = g_strdup (gtk_entry_get_text (GTK_ENTRY (tmp_widget)));
						else
							old_credit_payment_content = NULL;
					}

					gsb_payment_method_create_combo_list (widget,
														  GSB_PAYMENT_DEBIT,
														  account_number,
														  0,
														  FALSE);

					widget = gsb_form_widget_get_widget (TRANSACTION_FORM_CONTRA);
					if (widget && gtk_widget_get_visible (widget))
						gsb_payment_method_create_combo_list (gsb_form_widget_get_widget (TRANSACTION_FORM_CONTRA),
															  GSB_PAYMENT_CREDIT,
															  account_number,
															  0,
															  TRUE);
				}
			}
			gsb_form_check_auto_separator (entry);
			break;
		case TRANSACTION_FORM_CREDIT :
			 /* on met old_credit = NULl car avec g_free plantage */
			if (old_credit && strlen (old_credit) > 0)
				old_credit = NULL;

			/* save the debit if necessary */
			widget = gsb_form_widget_get_widget (TRANSACTION_FORM_DEBIT);
			if (!gsb_form_widget_check_empty (widget))
				old_debit = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));

			/* we change the payment method to adapt it for the debit */
			widget = gsb_form_widget_get_widget (TRANSACTION_FORM_TYPE);

			/* change the method of payment if necessary
			 * (if grey, it's a child of split so do nothing) */
			if (widget && gtk_widget_get_sensitive (widget))
			{
				/* change the signe of the method of payment and the contra */
				if (gsb_payment_method_get_combo_sign (widget) == GSB_PAYMENT_DEBIT)
				{
					old_debit_payment_number = gsb_payment_method_get_selected_number (widget);

					tmp_widget = gsb_form_widget_get_widget (TRANSACTION_FORM_CHEQUE);
					if (tmp_widget && gtk_widget_get_visible (tmp_widget))
					{
						if (old_debit_payment_content)
							g_free (old_debit_payment_content);
						if (gsb_form_widget_check_empty (tmp_widget) == FALSE)
							old_debit_payment_content = g_strdup (gtk_entry_get_text (GTK_ENTRY (tmp_widget)));
						else
							old_debit_payment_content = NULL;
					}

					gsb_payment_method_create_combo_list (widget,
														  GSB_PAYMENT_CREDIT,
														  account_number,
														  0,
														  FALSE);

					widget = gsb_form_widget_get_widget (TRANSACTION_FORM_CONTRA);
					if (widget && gtk_widget_get_visible (widget))
						gsb_payment_method_create_combo_list (widget,
															  GSB_PAYMENT_DEBIT,
															  account_number,
															  0,
															  TRUE);
				}
			}
			gsb_form_check_auto_separator (entry);
			break;
    }

    /* sensitive the valid and cancel buttons */
	gsb_form_sensitive_cancel_valid_buttons (TRUE);

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gchar *gsb_form_widget_get_old_credit (void)
{
    if (old_credit && strlen (old_credit) > 0)
        return g_strdup (old_credit);
    else
        return NULL;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gchar *gsb_form_widget_get_old_debit (void)
{
    if (old_debit && strlen (old_debit) > 0)
        return g_strdup (old_debit);
    else
        return NULL;
}

/**
 * called when entry changed
 * check the entry and set the entry red/invalid if not a good number
 *
 * \param entry
 * \param null
 *
 * \return FALSE
 **/
gboolean gsb_form_widget_amount_entry_changed (GtkWidget *entry,
                                               gpointer null)
{
    GtkStyleContext* context;
    gboolean valide;
    gint element_number;

    /* if we are in the form and the entry is empty, do nothing
     * because it's a special style too */
    element_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry), "element_number"));

    if (g_strcmp0 (gsb_form_widget_get_name (element_number),
                   gtk_entry_get_text (GTK_ENTRY (entry))) == 0)
        return FALSE;

    if (gsb_form_widget_check_empty (entry))
	    return FALSE;

    context = gtk_widget_get_style_context (entry);

    /* if nothing in the entry, keep the normal style */
    if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
    {
        gtk_widget_set_name (entry, "form_entry_empty");
        gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);

	    return FALSE;
    }

    valide = gsb_form_widget_get_valide_amout_entry (gtk_entry_get_text (GTK_ENTRY (entry)));
    if (valide)
    {
        /* the entry is valid, make it normal */
        gtk_widget_set_name (entry, "form_entry");
    }
    else
    {
	    /* the entry is not valid, make it red */
        gtk_widget_set_name (entry, "form_entry_error");
    }
    gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_form_widget_get_valide_amout_entry (const gchar *string)
{
    const gchar *ptr;
    gchar *mon_decimal_point;
    gunichar decimal_point;
    gchar *mon_thousands_sep;
    gunichar thousands_sep = (gunichar) -1;

    ptr = string;

    if ((g_utf8_strchr (ptr, -1, '*') && g_utf8_strchr (ptr, -1, '+'))
     ||
     (g_utf8_strchr (ptr, -1, '*') && g_utf8_strchr (ptr, -1, '-')))
        return FALSE;

    mon_decimal_point = gsb_locale_get_mon_decimal_point ();
    decimal_point = g_utf8_get_char_validated (mon_decimal_point, -1);
    mon_thousands_sep = gsb_locale_get_mon_thousands_sep ();
    if (mon_thousands_sep)
        thousands_sep = g_utf8_get_char_validated (mon_thousands_sep, -1);

    while (g_utf8_strlen (ptr, -1) > 0)
    {
        gunichar ch;

        ch = g_utf8_get_char_validated (ptr, -1);

        if (!g_unichar_isdefined (ch))
        {
            g_free (mon_decimal_point);
            if (mon_thousands_sep)
                g_free (mon_thousands_sep);

            return FALSE;
        }

        if (!g_ascii_isdigit (ch))
        {
			if (ch == 0x00A0 || ch == 0x202F/* NON-BREAK SPACE || ESPACE INSÉCABLE ÉTROITE */)
			{
				 ptr = g_utf8_next_char (ptr);
				continue;
			}
            else if (g_unichar_isdefined (thousands_sep))
            {
                if (ch != '.' && ch != ',' && ch != '+' && ch != '-'
                 && ch != '*' && ch != '/' && ch != thousands_sep)
                {
                    g_free (mon_decimal_point);
                    g_free (mon_thousands_sep);

                    return FALSE;
                }

                if (ch == decimal_point
                 && g_utf8_strlen (ptr, -1) == 1
                 && g_utf8_strchr (string, -1, thousands_sep))
                {
                    gchar **tab;
                    guint i = 1;    /* le premier champs peut etre < à 3 */
                    guint nbre_champs;

                    tab = g_strsplit (string, mon_thousands_sep, 0);
                    nbre_champs = g_strv_length (tab);

                    while (i < nbre_champs)
                    {
                        if (i < nbre_champs - 1 && g_utf8_strlen (tab[i], -1) != 3)
                        {
                            g_strfreev (tab);
                            g_free (mon_decimal_point);
                            g_free (mon_thousands_sep);

                            return FALSE;
                        }
                        else if (i == nbre_champs - 1 && g_utf8_strlen (tab[i], -1) != 4)
                        {
                            g_strfreev (tab);
                            g_free (mon_decimal_point);
                            g_free (mon_thousands_sep);

                            return FALSE;
                        }
                        i++;
                    }
                    g_strfreev (tab);
                }
            }
            else if (ch != '.' && ch != ',' && ch != '+' && ch != '-' && ch != '*' && ch != '/')
                return FALSE;
        }

        ptr = g_utf8_next_char (ptr);
    }

    g_free (mon_decimal_point);
    if (mon_thousands_sep)
        g_free (mon_thousands_sep);

    return TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_form_widget_get_old_credit_payment_number (void)
{
    return old_credit_payment_number;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_form_widget_get_old_debit_payment_number (void)
{
    return old_debit_payment_number;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_form_widget_get_old_credit_payment_content (void)
{
    return old_credit_payment_content;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
const gchar *gsb_form_widget_get_old_debit_payment_content (void)
{
    return old_debit_payment_content;
}

/**
 * called when entry validated by Enter Tab lose focus
 * check the entry and set the entry red/invalid if not a good number
 *
 * \param entry
 *
 * \return TRUE if entry is valid
 **/
gboolean gsb_form_widget_amount_entry_validate (gint element_number)
{
    GtkWidget *entry;
    GtkStyleContext* context;
    const gchar *text;
    gchar *tmp_str;
    gchar *mon_decimal_point;
    gboolean valide;
    gboolean return_value;

    entry = gsb_form_widget_get_widget (element_number);
    if (gsb_form_widget_check_empty (entry))
        return TRUE;

    text = gtk_entry_get_text (GTK_ENTRY (entry));
    if (g_strcmp0 (gsb_form_widget_get_name (element_number), text) == 0)
        return TRUE;

    /* if nothing in the entry, set the widget empty */
    if (!strlen (text))
    {
        gsb_form_widget_set_empty (entry, TRUE);
        return TRUE;
    }

    context = gtk_widget_get_style_context  (entry);

    mon_decimal_point = gsb_locale_get_mon_decimal_point ();
    if (g_strrstr (text, gsb_locale_get_mon_decimal_point ()) == NULL)
        tmp_str = g_strconcat (text, mon_decimal_point, NULL);
    else
        tmp_str= g_strdup (text);

    valide = gsb_form_widget_get_valide_amout_entry (tmp_str);
    if (valide)
    {
        /* the entry is valid, make it normal */
        gtk_widget_set_name (entry, "form_entry");
        return_value = TRUE;
    }
    else
    {
        /* the entry is not valid, make it red */
        gtk_widget_set_name (entry, "form_entry_error");
        return_value = FALSE;
    }

    gtk_style_context_set_state (context, GTK_STATE_FLAG_NORMAL);

    g_free (tmp_str);
    g_free (mon_decimal_point);

    return return_value;
}

/**
 * update the payee combofix in the form with the current current report
 *
 * \param report_number
 * \param sens              FALSE remove report TRUE add report
 *
 * \return FALSE
 **/
gboolean gsb_form_widget_update_payee_combofix (gint report_number,
												gint sens)
{
    if (sens)
        gtk_combofix_append_report (GTK_COMBOFIX (gsb_form_widget_get_widget
												  (TRANSACTION_FORM_PARTY)),
									gsb_data_report_get_report_name (report_number));
    else
        gtk_combofix_remove_report (GTK_COMBOFIX (gsb_form_widget_get_widget
												  (TRANSACTION_FORM_PARTY)),
									gsb_data_report_get_report_name (report_number));

    return FALSE;
}

/**
 * free the content of the list of structures of the form
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_form_widget_free_list_without_widgets (void)
{
	GSList *tmp_list;

    devel_debug (NULL);

    if (!form_list_widgets)
        return FALSE;

    tmp_list = form_list_widgets;
    while (tmp_list)
    {
        FormElement *element;

        element = tmp_list->data;
        g_free (element);
        tmp_list = tmp_list->next;
    }
    g_slist_free (form_list_widgets);
    form_list_widgets = NULL;

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

