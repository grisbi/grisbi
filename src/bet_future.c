/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C) 2007 Dominique Parisot                                   */
/*          zionly@free.org                                                   */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                 */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "bet_future.h"
#include "bet_tab.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_calendar_entry.h"
#include "gsb_combo_box.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency_link.h"
#include "gsb_data_form.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_form.h"
#include "gsb_form_scheduler.h"
#include "gsb_form_widget.h"
#include "gsb_fyear.h"
#include "gsb_payment_method.h"
#include "gtk_combofix.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_editables.h"
#include "utils_real.h"
#include "utils_str.h"
#include "widget_transfer.h"
#include "erreur.h"
/*END_INCLUDE*/

/* dimmension du formulaire de données futures */
#define BET_FUTURE_FORM_WIDTH 4
#define BET_FUTURE_FORM_HEIGHT 5

typedef struct _BetFormOrganization			BetFormOrganization;

struct _BetFormOrganization
{
    /* 4 columns */
    gint columns;
    /* 5 rows */
    gint rows;
    gint form_table[BET_FUTURE_FORM_HEIGHT][BET_FUTURE_FORM_WIDTH];
};

/*START_STATIC*/
/** contains the list of the scheduled elements, ie list of link
 * between an element number and the pointer of its widget
 * for now, this list if filled at the opening of grisbi and never erased */
static GSList *bet_schedul_element_list = NULL;

/** contains a list of FormElement according to the current form */
static GSList *bet_form_list_widgets = NULL;

static GtkWidget *bet_futur_dialog = NULL;
static GtkWidget *bet_transfert_dialog = NULL;

/* organisation du formulaire */
static BetFormOrganization *bfo_struct;
/*END_STATIC*/

/* BET_FORM */
/******************************************************************************/
/* Private Methods                                                            */
/******************************************************************************/
/**
 * return the widget of the element_number given in param,
 * for the bet scheduler part of the form
 *
 * \param element_number
 *
 * \return a GtkWidget * or NULL
 **/
static GtkWidget *bet_form_scheduler_get_element_widget (gint element_number)
{
    return gsb_form_get_element_widget_from_list (element_number,
                        bet_schedul_element_list);
}

/**
 * return the pointer to the widget corresponding to the given element
 *
 * \param element_number
 *
 * \return the widget or NULL
 **/
static GtkWidget *bet_form_widget_get_widget (gint element_number)
{
    return gsb_form_get_element_widget_from_list (element_number, bet_form_list_widgets);
}

/**
 * called when we press the button in an entry field in
 * the form
 *
 * \param entry which receive the signal
 * \param ev can be NULL
 * \param ptr_origin a pointer to int on the element_number
 *
 * \return FALSE
 **/
static gboolean bet_form_button_press_event (GtkWidget *entry,
											 GdkEventButton *ev,
											 gint *ptr_origin)
{
    GtkWidget *date_entry;

	/* set the current date into the date entry */
	date_entry = bet_form_widget_get_widget (TRANSACTION_FORM_DATE);
	if (gsb_form_widget_check_empty (date_entry))
	{
        gtk_entry_set_text (GTK_ENTRY (date_entry), gsb_date_today ());
	    gsb_form_widget_set_empty (date_entry, FALSE);
	}

    return FALSE;
}

/**
 * check if the given element can receive focus
 *
 * \param element_number
 * \param
 *
 * \return TRUE : can receive focus, or FALSE
 **/
static gboolean bet_form_widget_can_focus (gint element_number,
										   gint form_type)
{
    GtkWidget *widget;

    /* if element_number is -1 or -2, the iteration while must
     * stop, so return TRUE */
    if (element_number == -1 || element_number == -2)
        return TRUE;

    if (form_type)
        widget = bet_form_scheduler_get_element_widget (element_number);
    else
        widget = bet_form_widget_get_widget (element_number);

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
        return FALSE;

    return TRUE;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static GtkWidget *bet_form_widget_get_next_element (gint element_number,
													gint form_type)
{
    gint row = 0;
    gint column = 0;
    gint return_value_number = 0;
    gint form_column_number = BET_FUTURE_FORM_WIDTH;
    gint form_row_number = 5;
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

    if (form_type) /* le widget initial est sur la partie scheduled du formulaire */
    {
        for (column=0 ; column < bfo_struct->columns ; column++)
        {
            if (bfo_struct->form_table[row][column] == element_number)
            {
                form_row_number = 1;
                while (!bet_form_widget_can_focus (return_value_number, form_type))
                {
                    if (column == (form_column_number - 1)
                         &&
                         row == (form_row_number - 1))
                    {
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
                    return_value_number = bfo_struct->form_table[row][column];
                }
                break;
            }
        }
    }
    else
    {
        gboolean trouve = FALSE;
        for (row=1 ; row < bfo_struct->rows; row++)
        {
            for (column=0 ; column < bfo_struct->columns ; column++)
            {
                if (bfo_struct->form_table[row][column] == element_number)
                {
                    trouve = TRUE;
                    break;
                }
            }
            if (trouve)
                break;
        }
        while (!bet_form_widget_can_focus (return_value_number, form_type))
        {
            if (column == (form_column_number - 1)
                 &&
                 row == (form_row_number - 1))
            {
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
            return_value_number = bfo_struct->form_table[row][column];
        }
    }

    /* return value */
    if (row == 0)
        return gsb_form_get_element_widget_from_list (return_value_number,
													  bet_schedul_element_list);
    else
        return gsb_form_get_element_widget_from_list (return_value_number,
													  bet_form_list_widgets);
}

/**
 * called when the frequency button is changed
 * show/hide the necessary widget according to its state
 *
 * \param combo_box
 * \param
 *
 * \return FALSE
 **/
static gboolean bet_form_scheduler_frequency_button_changed (GtkWidget *combo_box,
															 GtkWidget *dialog)
{
    gchar *selected_item;

	selected_item = gsb_combo_box_get_active_text (GTK_COMBO_BOX (combo_box));

    if (!strcmp (selected_item, _("Once")))
    {
        gtk_widget_hide (bet_form_scheduler_get_element_widget (SCHEDULED_FORM_LIMIT_DATE));
        gtk_widget_hide (bet_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_ENTRY));
        gtk_widget_hide (bet_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_BUTTON));
    }
    else
    {
        gtk_widget_show (bet_form_scheduler_get_element_widget (SCHEDULED_FORM_LIMIT_DATE));

        if (!strcmp (selected_item, _("Custom")))
        {
            gtk_widget_show (bet_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_ENTRY));
            gtk_widget_show (bet_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_BUTTON));
        }
        else
        {
            gtk_widget_hide (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_ENTRY));
            gtk_widget_hide (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_BUTTON));
        }
    }
    g_free (selected_item);
    gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);

    return FALSE;
}

/**
 * called when press a key on an element of the form
 *
 * \param widget which receive the signal
 * \param ev
 * \param ptr_origin a pointer number of the element
 *
 * \return FALSE
 **/
static gboolean bet_form_key_press_event (GtkWidget *widget,
										  GdkEventKey *ev,
										  gint *ptr_origin)
{
    GtkWidget *widget_suivant;
    GtkWidget *widget_prov;
    gint element_number;
    gint form_type = 0;
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

    form_type = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (widget), "form_type"));

    /* on sort si on est sur la partie scheduler du formulaire sauf pour GDK_Tab */
    if (form_type && ev->keyval != GDK_KEY_Tab)
        return FALSE;

    element_number = GPOINTER_TO_INT (ptr_origin);

    /* if form_key_entree = 1, entry finish the transaction, else does as tab */
    if (!a_conf->form_enter_key && (ev->keyval == GDK_KEY_Return || ev->keyval == GDK_KEY_KP_Enter))
		ev->keyval = GDK_KEY_Tab ;

    switch (ev->keyval)
    {
    case GDK_KEY_1:
    case GDK_KEY_2:
    case GDK_KEY_3:
    case GDK_KEY_4:
    case GDK_KEY_5:
    case GDK_KEY_6:
    case GDK_KEY_7:
    case GDK_KEY_8:
    case GDK_KEY_9:
    case GDK_KEY_0:
        switch (element_number)
        {
        case TRANSACTION_FORM_DEBIT:
            widget_prov = bet_form_widget_get_widget (TRANSACTION_FORM_CREDIT);
            if (!gsb_form_widget_check_empty (widget_prov))
            {
                gtk_entry_set_text (GTK_ENTRY (widget_prov),
									gsb_form_widget_get_name (TRANSACTION_FORM_CREDIT));
                gsb_form_widget_set_empty (widget_prov, TRUE);
            }
            break;
        case TRANSACTION_FORM_CREDIT:
            widget_prov = bet_form_widget_get_widget (TRANSACTION_FORM_DEBIT);
            if (!gsb_form_widget_check_empty (widget_prov))
            {
                gtk_entry_set_text (GTK_ENTRY (widget_prov),
									gsb_form_widget_get_name (TRANSACTION_FORM_DEBIT));
                gsb_form_widget_set_empty (widget_prov, TRUE);
            }
            break;
        }
        break;
	case GDK_KEY_Escape :
	    gsb_form_escape_form ();
	    break;

	case GDK_KEY_KP_Enter :
	case GDK_KEY_Return :

	    break;

	case GDK_KEY_KP_Add:
	case GDK_KEY_plus:
	case GDK_KEY_equal:		/* This should make all our US users happy */

	    /* increase the check of 1 */
	    if (element_number == TRANSACTION_FORM_CHEQUE)
	    {
			increment_decrement_champ (widget, 1);

			return TRUE;
	    }
	    break;

	case GDK_KEY_KP_Subtract:
	case GDK_KEY_minus:

	    /* decrease the check of 1 */
	    if (element_number == TRANSACTION_FORM_CHEQUE)
	    {
			increment_decrement_champ (widget, -1);

			return TRUE;
	    }
	    break;

    case GDK_KEY_Tab :
        widget_suivant = bet_form_widget_get_next_element (element_number, form_type);

        if (widget_suivant)
        {
            if (GTK_IS_COMBOFIX (widget_suivant))
                gtk_widget_grab_focus (gtk_combofix_get_entry (GTK_COMBOFIX (widget_suivant)));
            else
                gtk_widget_grab_focus (widget_suivant);

            return TRUE;
        }
        break;
    }

    return FALSE;
}

/**
 * clean the form according to the account_number
 * and set the default values
 *
 * \param account number
 *
 * \return FALSE
 **/
static gboolean bet_form_clean (gint account_number)
{
    GSList *tmp_list;

    /* clean the scheduled widget */
    tmp_list = bet_schedul_element_list;

    while (tmp_list)
    {
        FormElement *element;

        element = tmp_list->data;

        /* better to protect here if widget != NULL (bad experience...) */
        if (element->element_widget)
        {
            gtk_widget_set_sensitive (element->element_widget, TRUE);

            switch (element->element_number)
            {
            case SCHEDULED_FORM_FREQUENCY_BUTTON:
                gsb_combo_box_set_index (element->element_widget, 0);
                break;
            case SCHEDULED_FORM_LIMIT_DATE:
                gsb_form_widget_set_empty (element->element_widget, TRUE);
                gtk_entry_set_text (GTK_ENTRY (element->element_widget), _("Limit date"));
                break;
            case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
                gtk_entry_set_text (GTK_ENTRY (element->element_widget), _("Own frequency"));
                gsb_form_widget_set_empty (element->element_widget, TRUE);
                break;
    		case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
            gsb_combo_box_set_index (element->element_widget, 2);
		    break;
            }
        }

        tmp_list = tmp_list->next;
    }

    /* clean the transactions widget */
    tmp_list = bet_form_list_widgets;

    while (tmp_list)
    {
        FormElement *element;

        element = tmp_list->data;

        /* better to protect here if widget != NULL (bad experience...) */
        if (element->element_widget)
        {
            /* some widgets can be set unsensitive because of the children of splits,
             * so resensitive all to be sure */
            gtk_widget_set_sensitive (element->element_widget, TRUE);

            switch (element->element_number)
            {
				case TRANSACTION_FORM_DATE:
					if (!strlen (gtk_entry_get_text (GTK_ENTRY (element->element_widget))))
					{
						gsb_form_widget_set_empty (element->element_widget, TRUE);
						gtk_entry_set_text (GTK_ENTRY (element->element_widget), _("Date"));
					}
					break;

				case TRANSACTION_FORM_EXERCICE:
					/* editing a transaction can show some fyear which shouldn't be showed,
					 * so hide them here */
					gsb_fyear_update_fyear_list ();

					/* set the combo_box on 'Automatic' */
					gsb_fyear_set_combobox_history (element->element_widget, 0);

					break;

				case TRANSACTION_FORM_PARTY:
					gsb_form_widget_set_empty (element->element_widget, TRUE);
					gtk_combofix_set_text (GTK_COMBOFIX (element->element_widget), _("Payee"));
					break;

				case TRANSACTION_FORM_DEBIT:
					gsb_form_widget_set_empty (element->element_widget, TRUE);
					gtk_entry_set_text (GTK_ENTRY (element->element_widget), _("Debit"));
					break;

				case TRANSACTION_FORM_CREDIT:
					gsb_form_widget_set_empty (element->element_widget, TRUE);
					gtk_entry_set_text (GTK_ENTRY (element->element_widget), _("Credit"));
					break;

				case TRANSACTION_FORM_CATEGORY:
				   gsb_form_widget_set_empty (element->element_widget, TRUE);
					gtk_combofix_set_text (GTK_COMBOFIX (element->element_widget),
										   _("Categories : Sub-categories"));
					break;

				case TRANSACTION_FORM_BUDGET:
						gsb_form_widget_set_empty (element->element_widget, TRUE);
						gtk_combofix_set_text (GTK_COMBOFIX (element->element_widget),
											   _("Budgetary line"));
					break;

				case TRANSACTION_FORM_NOTES:
					gsb_form_widget_set_empty (element->element_widget, TRUE);
					gtk_entry_set_text (GTK_ENTRY (element->element_widget), _("Notes"));
					break;

				case TRANSACTION_FORM_TYPE:
					gsb_payment_method_set_combobox_history (element->element_widget,
															 gsb_data_account_get_default_debit (account_number),
															 TRUE);
					break;

			}
        }
        tmp_list = tmp_list->next;
    }

    return FALSE;
}

/**
 * called when an entry lose the focus
 *
 * \param entry
 * \param ev
 * \param ptr_origin a pointer gint which is the number of the element
 *
 * \return FALSE
 **/
static gboolean bet_form_entry_lose_focus (GtkWidget *entry,
										   GdkEventFocus *ev,
										   gint *ptr_origin)
{
    GtkWidget *widget;
    const gchar *string;
    gint element_number;
    gint account_number;

    /* still not found, if change the content of the form, something come in entry
     * which is nothing, so protect here */
    if (!GTK_IS_WIDGET (entry) || !GTK_IS_ENTRY (entry))
		return FALSE;

    /* remove the selection */
    gtk_editable_select_region (GTK_EDITABLE (entry), 0, 0);
    element_number = GPOINTER_TO_INT (ptr_origin);
    account_number = gsb_form_get_account_number ();

    /* sometimes the combofix popus stays showed, so remove here */
    if (element_number == TRANSACTION_FORM_PARTY
		|| element_number == TRANSACTION_FORM_CATEGORY
		|| element_number == TRANSACTION_FORM_BUDGET)
    {
        widget = bet_form_widget_get_widget (element_number);
        gtk_combofix_hide_popup (GTK_COMBOFIX (widget));
    }

    /* string will be filled only if the field is empty */
    string = NULL;
    switch (element_number)
    {
		case TRANSACTION_FORM_DATE :
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
				string = gsb_form_widget_get_name (TRANSACTION_FORM_DATE);
			break;

		case TRANSACTION_FORM_PARTY :
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
				string = gsb_form_widget_get_name (TRANSACTION_FORM_PARTY);
			break;

		case TRANSACTION_FORM_DEBIT :
			/* we change the payment method to adapt it for the debit */
			if (strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				/* empty the credit */
				widget = bet_form_widget_get_widget (TRANSACTION_FORM_CREDIT);
				if (!gsb_form_widget_check_empty (widget))
				{
					gtk_entry_set_text (GTK_ENTRY (widget),
										gsb_form_widget_get_name (TRANSACTION_FORM_CREDIT));
					gsb_form_widget_set_empty (widget, TRUE);
				}

				widget = bet_form_widget_get_widget (TRANSACTION_FORM_TYPE);

				/* change the method of payment if necessary
				 * (if grey, it's a child of split so do nothing) */
				if (widget && gtk_widget_get_sensitive (widget))
				{
					/* change the signe of the method of payment and the contra */
					if (gsb_payment_method_get_combo_sign (widget) == GSB_PAYMENT_CREDIT)
					{
						gsb_payment_method_create_combo_list (widget,
															  GSB_PAYMENT_DEBIT,
															  account_number,
															  0,
															  FALSE);
					}
				}
				gsb_form_check_auto_separator (entry);
			}
			else
			{
				/* si pas de nouveau débit on essaie de remettre l'ancien crédit */
				gchar *vstring;

				vstring = gsb_form_widget_get_old_credit ();
				if (vstring)
				{
					GtkWidget *widget_prov;

					widget_prov = bet_form_widget_get_widget (TRANSACTION_FORM_CREDIT);

					gtk_entry_set_text (GTK_ENTRY (widget_prov), vstring);
					gsb_form_widget_set_empty (widget_prov, FALSE);
					g_free (vstring);

					widget = bet_form_widget_get_widget (TRANSACTION_FORM_TYPE);
					if (widget && gtk_widget_get_sensitive (widget))
					{
						/* change the signe of the method of payment and the contra */
						if (gsb_payment_method_get_combo_sign (widget) == GSB_PAYMENT_DEBIT)
						{
							gsb_payment_method_create_combo_list (widget,
																  GSB_PAYMENT_CREDIT,
																  account_number,
																  0,
																  FALSE);
						}
					}
				}
				string = gsb_form_widget_get_name (TRANSACTION_FORM_DEBIT);
			}
			break;

		case TRANSACTION_FORM_CREDIT :
			/* we change the payment method to adapt it for the debit */
			if (strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
			{
				/* empty the credit */
				widget = bet_form_widget_get_widget (TRANSACTION_FORM_DEBIT);
				if (!gsb_form_widget_check_empty (widget))
				{
					gtk_entry_set_text (GTK_ENTRY (widget),
										gsb_form_widget_get_name (TRANSACTION_FORM_DEBIT));
					gsb_form_widget_set_empty (widget, TRUE);
				}
				widget = bet_form_widget_get_widget (TRANSACTION_FORM_TYPE);

				/* change the method of payment if necessary
				 * (if grey, it's a child of split so do nothing) */
				if (widget && gtk_widget_get_sensitive (widget))
				{
					/* change the signe of the method of payment and the contra */
					if (gsb_payment_method_get_combo_sign (widget) == GSB_PAYMENT_DEBIT)
					{
						gsb_payment_method_create_combo_list (widget,
															  GSB_PAYMENT_CREDIT,
															  account_number,
															  0,
															  FALSE);
					}
				}
				gsb_form_check_auto_separator (entry);
			}
			else
			{
				/* si pas de nouveau credit on essaie de remettre l'ancien débit */
				gchar *vstring;

				vstring = gsb_form_widget_get_old_debit ();
				if (vstring)
				{
					GtkWidget * widget_prov;

					widget_prov = bet_form_widget_get_widget (TRANSACTION_FORM_DEBIT);

					gtk_entry_set_text (GTK_ENTRY (widget_prov), vstring);
					gsb_form_widget_set_empty (widget_prov, FALSE);
					g_free (vstring);

					widget = bet_form_widget_get_widget (TRANSACTION_FORM_TYPE);
					if (widget && gtk_widget_get_sensitive (widget))
					{
						/* change the signe of the method of payment and the contra */
						if (gsb_payment_method_get_combo_sign (widget) == GSB_PAYMENT_CREDIT)
						{
							gsb_payment_method_create_combo_list (widget,
																  GSB_PAYMENT_DEBIT,
																  account_number,
																  0,
																  FALSE);
						}
					}
				}
				string = gsb_form_widget_get_name (TRANSACTION_FORM_CREDIT);
			}
			break;

		case TRANSACTION_FORM_CATEGORY :
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
				string = gsb_form_widget_get_name (TRANSACTION_FORM_CATEGORY);
			break;

		case TRANSACTION_FORM_BUDGET :
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
				string = gsb_form_widget_get_name (TRANSACTION_FORM_BUDGET);
			break;

		case TRANSACTION_FORM_NOTES :
			if (!strlen (gtk_entry_get_text (GTK_ENTRY (entry))))
				string = _(gsb_form_widget_get_name (element_number));
			break;

		default :
			break;
    }

    /* if string is not NULL, the entry is empty so set the empty field to TRUE */
    if (string)
    {
        switch (element_number)
        {
            case TRANSACTION_FORM_PARTY :
            case TRANSACTION_FORM_CATEGORY :
            case TRANSACTION_FORM_BUDGET :
				/* need to work with the combofix to avoid some signals if we work
				 * directly on the entry */
				gtk_combofix_set_text (GTK_COMBOFIX (bet_form_widget_get_widget (element_number)),
									   _(string));
				break;

            default:
                gtk_entry_set_text (GTK_ENTRY (entry), string);
            break;
        }
        gsb_form_widget_set_empty (entry, TRUE);
    }

    return FALSE;
}

/**
 * fill the form according to the account_number :
 *
 * \param account_number the number of account
 *
 * \return FALSE
 **/
static gboolean bet_form_create_current_form (GtkWidget *dialog,
											  GtkWidget *table,
											  gint account_number)
{
	GtkWidget *widget;
    gint element_number;
    gint row = 2;
    gint column = 0;
    FormElement *element;
    GSList *tmp_list;

    account_number = gsb_gui_navigation_get_current_account ();

    element_number = TRANSACTION_FORM_DATE;
    widget = gsb_calendar_entry_new (FALSE);
    gtk_widget_show (widget);
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 (sizeof (FormElement));
    element->element_number = element_number;
    element->element_widget = widget;
    bet_form_list_widgets = g_slist_append (bet_form_list_widgets, element);
    column ++;

    element_number = TRANSACTION_FORM_PARTY;
	tmp_list = gsb_data_payee_get_name_and_report_list ();
	widget = gtk_combofix_new_with_properties (tmp_list,
											   TRUE,							/* on ne peut pas créer d'item */
											   etat.combofix_case_sensitive,
											   FALSE,
											   METATREE_PAYEE);
	gsb_data_payee_free_name_and_report_list (tmp_list);
    gtk_widget_show (widget);
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 (sizeof (FormElement));
    element->element_number = element_number;
    element->element_widget = widget;
    bet_form_list_widgets = g_slist_append (bet_form_list_widgets, element);
    column ++;

    element_number = TRANSACTION_FORM_DEBIT;
    widget = gtk_entry_new ();
    g_object_set_data (G_OBJECT (widget), "element_number",
                        GINT_TO_POINTER (TRANSACTION_FORM_DEBIT));
    g_signal_connect (G_OBJECT (widget),
					  "changed",
		              G_CALLBACK (gsb_form_widget_amount_entry_changed),
					  NULL);
    gtk_widget_show (widget);
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 (sizeof (FormElement));
    element->element_number = element_number;
    element->element_widget = widget;
    bet_form_list_widgets = g_slist_append (bet_form_list_widgets, element);
    column ++;

    element_number = TRANSACTION_FORM_CREDIT;
    widget = gtk_entry_new ();
    g_object_set_data (G_OBJECT (widget), "element_number",
                        GINT_TO_POINTER (TRANSACTION_FORM_CREDIT));
    g_signal_connect (G_OBJECT (widget),
		              "changed",
		              G_CALLBACK (gsb_form_widget_amount_entry_changed),
		              NULL);
    gtk_widget_show (widget);
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 (sizeof (FormElement));
    element->element_number = element_number;
    element->element_widget = widget;
    bet_form_list_widgets = g_slist_append (bet_form_list_widgets, element);
    column = 0;
    row ++;

    element_number = TRANSACTION_FORM_EXERCICE;
    widget = gsb_fyear_make_combobox (TRUE);
    gtk_widget_set_tooltip_text (GTK_WIDGET (widget),
					  _("Choose the financial year"));
    gtk_widget_show (widget);
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 (sizeof (FormElement));
    element->element_number = element_number;
    element->element_widget = widget;
    bet_form_list_widgets = g_slist_append (bet_form_list_widgets, element);
    column ++;

    element_number = TRANSACTION_FORM_CATEGORY;
	tmp_list = gsb_data_category_get_name_list (TRUE, TRUE, TRUE, TRUE);
	widget = gtk_combofix_new_with_properties (tmp_list,
											   TRUE,							/* on ne peut pas créer d'item */
											   etat.combofix_case_sensitive,
											   etat.combofix_mixed_sort,
											   METATREE_CATEGORY);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_widget_show (widget);
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 (sizeof (FormElement));
    element->element_number = element_number;
    element->element_widget = widget;
    bet_form_list_widgets = g_slist_append (bet_form_list_widgets, element);
    column ++;

    element_number = TRANSACTION_FORM_TYPE;
    widget = gtk_combo_box_new ();
    gsb_payment_method_create_combo_list (widget,
                        GSB_PAYMENT_DEBIT,
                        account_number, 0, FALSE);
    gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 0);
    gtk_widget_set_tooltip_text (GTK_WIDGET (widget), _("Choose the method of payment"));
    gtk_widget_show (widget);
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);

    element = g_malloc0 (sizeof (FormElement));
    element->element_number = element_number;
    element->element_widget = widget;
    bet_form_list_widgets = g_slist_append (bet_form_list_widgets, element);
    column = 1;
    row ++;

    element_number = TRANSACTION_FORM_BUDGET;
	tmp_list = gsb_data_budget_get_name_list (TRUE, TRUE);
	widget = gtk_combofix_new_with_properties (tmp_list,
											   TRUE,							/* on ne peut pas créer d'item */
											   etat.combofix_case_sensitive,
											   etat.combofix_mixed_sort,
											   METATREE_BUDGET);
	gsb_data_categorie_free_name_list (tmp_list);
    gtk_widget_show (widget);
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 1, 1);
    element = g_malloc0 (sizeof (FormElement));
    element->element_number = element_number;
    element->element_widget = widget;
    bet_form_list_widgets = g_slist_append (bet_form_list_widgets, element);
    column = 0;
    row ++;

    element_number = TRANSACTION_FORM_NOTES;
	widget = gtk_entry_new();
    gtk_widget_show (widget);
    gtk_grid_attach (GTK_GRID (table), widget, column, row, 4, 1);
    element = g_malloc0 (sizeof (FormElement));
    element->element_number = element_number;
    element->element_widget = widget;
    bet_form_list_widgets = g_slist_append (bet_form_list_widgets, element);

    tmp_list = bet_form_list_widgets;

    while (tmp_list)
    {
        FormElement *element_list;

        element_list = tmp_list->data;

		widget = element_list->element_widget;
        if (GTK_IS_ENTRY (widget))
        {
            g_signal_connect (G_OBJECT (widget),
							  "focus-in-event",
							  G_CALLBACK (bet_form_entry_get_focus),
							  dialog);
            g_signal_connect (G_OBJECT (widget),
							  "focus-out-event",
							  G_CALLBACK (bet_form_entry_lose_focus),
							  GINT_TO_POINTER (element_list->element_number));
            g_signal_connect (G_OBJECT (widget),
							  "button-press-event",
							  G_CALLBACK (bet_form_button_press_event),
							  GINT_TO_POINTER (element_list->element_number));
            g_signal_connect (G_OBJECT (widget),
							  "key-press-event",
							  G_CALLBACK (bet_form_key_press_event),
							  GINT_TO_POINTER (element_list->element_number));
        }
        else
        {
            if (GTK_IS_COMBOFIX (widget))
            {
				GtkWidget *entry;

				entry = gtk_combofix_get_entry (GTK_COMBOFIX (widget));
                g_signal_connect (G_OBJECT (entry),
								  "focus-in-event",
								  G_CALLBACK (bet_form_entry_get_focus),
								  dialog);
                g_signal_connect (G_OBJECT (entry),
								  "focus-out-event",
								  G_CALLBACK (bet_form_entry_lose_focus),
								  GINT_TO_POINTER (element_list->element_number));
                g_signal_connect (G_OBJECT (entry),
								  "button-press-event",
								  G_CALLBACK (bet_form_button_press_event),
								  GINT_TO_POINTER (element_list->element_number));
                g_signal_connect (G_OBJECT (entry),
								  "key-press-event",
								  G_CALLBACK (bet_form_key_press_event),
								  GINT_TO_POINTER (element_list->element_number));
            }
            else
            /* neither an entry, neither a combofix */
            g_signal_connect (G_OBJECT (widget),
							  "key-press-event",
							  G_CALLBACK (bet_form_key_press_event),
							  GINT_TO_POINTER (element_list->element_number));

            g_object_set_data (G_OBJECT (widget), "form_type", GINT_TO_POINTER (0));
        }
        tmp_list = tmp_list->next;
    }

    bet_form_clean (account_number);

    return FALSE;
}

/**
 * create the scheduled part
 *
 * \param table a GtkTable with the dimension SCHEDULED_HEIGHT*SCHEDULED_WIDTH to be filled
 * \param
 *
 * \return FALSE
 **/
static gboolean bet_form_create_scheduler_part (GtkWidget *dialog,
												GtkWidget *table)
{

    GtkWidget *separator;
    GtkWidget *combo = NULL;
    gint column;

    devel_debug (NULL);
	if (!table)
		return FALSE;

    /* ok, now fill the form
     * we play with height and width, but for now it's fix : 6 columns and 1 line */
	for (column=0 ; column < SCHEDULED_WIDTH ; column++)
	{
        GtkWidget *widget = NULL;
	    const gchar *tooltip_text = NULL;
	    const gchar *text_frequency [] = {N_("Once"), N_("Weekly"), N_("Monthly"),N_("Bimonthly"),
										  N_("Quarterly"), N_("Yearly"), N_("Custom"), NULL };
	    const gchar *text_frequency_user [] = { N_("Days"), N_("Weeks"), N_("Months"), N_("Years"), NULL };
        gint element_number;
        FormElement *element;

        /* on tient compte que le premier widget utile est le troisième du formulaire */
	    element_number = column + 2;

	    switch (element_number)
	    {
		case SCHEDULED_FORM_FREQUENCY_BUTTON:
		    widget = gsb_combo_box_new_with_index (text_frequency,
												   G_CALLBACK (bet_form_scheduler_frequency_button_changed),
												   dialog);
            g_object_set_data (G_OBJECT (widget), "form_type", GINT_TO_POINTER (1));
            combo = widget;
		    tooltip_text = _("Frequency");
            gtk_widget_show (widget);
		    break;

        case SCHEDULED_FORM_LIMIT_DATE:
            widget = gsb_calendar_entry_new (FALSE);
            g_object_set_data (G_OBJECT (widget), "form_type", GINT_TO_POINTER (1));
            gsb_form_widget_set_empty (widget, TRUE);
            gtk_entry_set_text (GTK_ENTRY (widget), _("Limit date"));
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
            g_signal_connect (G_OBJECT (widget),
                       	  "key-press-event",
                       	  G_CALLBACK (bet_form_key_press_event),
                       	  GINT_TO_POINTER (element_number));
            tooltip_text = _("Final date");
            gtk_widget_show (widget);
            break;

		case SCHEDULED_FORM_FREQUENCY_USER_ENTRY:
		    widget = gtk_entry_new ();
            g_object_set_data (G_OBJECT (widget), "form_type", GINT_TO_POINTER (1));
            gtk_entry_set_text (GTK_ENTRY (widget), _("Own frequency"));
            gsb_form_widget_set_empty (widget, TRUE);
            g_signal_connect (G_OBJECT (widget),
							  "focus-in-event",
                        	  G_CALLBACK (gsb_form_entry_get_focus),
                        	  GINT_TO_POINTER (element_number));
            g_signal_connect_after (G_OBJECT (widget),
                        	 		"focus-out-event",
                        	  		G_CALLBACK (gsb_form_scheduler_entry_lose_focus),
                        			GINT_TO_POINTER (element_number));
            g_signal_connect (G_OBJECT (widget),
                       		  "key-press-event",
                       		  G_CALLBACK (bet_form_key_press_event),
                       		  GINT_TO_POINTER (element_number));
		    tooltip_text = _("Custom frequency");
		    break;

		case SCHEDULED_FORM_FREQUENCY_USER_BUTTON:
		    widget = gsb_combo_box_new_with_index (text_frequency_user, NULL, NULL);
            g_object_set_data (G_OBJECT (widget), "form_type", GINT_TO_POINTER (1));
            g_signal_connect (G_OBJECT (widget),
							  "key-press-event",
							  G_CALLBACK (bet_form_key_press_event),
							  GINT_TO_POINTER (element_number));
		    tooltip_text = _("Custom frequency");
            gsb_combo_box_set_index (widget, 2);
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
	    bet_schedul_element_list = g_slist_append (bet_schedul_element_list, element);

	    /* set in the form */
        gtk_grid_attach (GTK_GRID (table), widget, column, 0, 1, 1);
	}

    separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_show (separator);
    gtk_grid_attach (GTK_GRID (table), separator, 0, 1, 4, 1);

    gsb_combo_box_set_index (combo, 0);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void bet_form_set_organization (void)
{
    gint tab[BET_FUTURE_FORM_HEIGHT][BET_FUTURE_FORM_WIDTH] = {
        { SCHEDULED_FORM_FREQUENCY_BUTTON, SCHEDULED_FORM_LIMIT_DATE, SCHEDULED_FORM_FREQUENCY_USER_ENTRY,
          SCHEDULED_FORM_FREQUENCY_USER_BUTTON },
        { TRANSACTION_FORM_DATE, TRANSACTION_FORM_PARTY, TRANSACTION_FORM_DEBIT, TRANSACTION_FORM_CREDIT },
        { TRANSACTION_FORM_EXERCICE, TRANSACTION_FORM_CATEGORY, TRANSACTION_FORM_TYPE, 0 },
        { 0, TRANSACTION_FORM_BUDGET, 0, 0 },
        { TRANSACTION_FORM_NOTES, 0, 0, 0 }
    };
    gint i, j;

	bfo_struct = g_malloc0 (sizeof (BetFormOrganization));

	if (!bfo_struct)
	{
		dialogue_error_memory ();
		return;
	}

	bfo_struct->columns = BET_FUTURE_FORM_WIDTH;
	bfo_struct->rows = BET_FUTURE_FORM_HEIGHT;

	for (i = 0 ; i<BET_FUTURE_FORM_HEIGHT ; i++)
	for (j = 0 ; j<BET_FUTURE_FORM_WIDTH ; j++)
		bfo_struct->form_table[i][j] = tab[i][j];
}

/******************************************************************************/
/* Public Methods                                                             */
/******************************************************************************/
/**
 * called when an entry get the focus, if the entry is free,
 * set it normal and erase the help content
 *
 * \param entry
 * \param
 * \param
 *
 * \return FALSE
 **/
gboolean bet_form_entry_get_focus (GtkWidget *entry,
										  GdkEventFocus *ev,
										  GtkWidget *dialog)
{
    /* the entry can be a combofix or a real entry */
    if (GTK_IS_COMBOFIX (entry))
    {
        if (gsb_form_widget_check_empty (entry))
        {
            gtk_combofix_set_text (GTK_COMBOFIX (entry), "");
            gsb_form_widget_set_empty (entry, FALSE);
        }
    }
    else
    {
        if (gsb_form_widget_check_empty (entry))
        {
            gtk_entry_set_text (GTK_ENTRY (entry), "");
            gsb_form_widget_set_empty (entry, FALSE);
        }
    }
    /* sensitive the valid button */
    if (dialog && GTK_IS_DIALOG (dialog))
        gtk_dialog_set_response_sensitive (GTK_DIALOG (dialog), GTK_RESPONSE_OK, TRUE);

    return FALSE;
}

/* BET_UTILS */
/******************************************************************************/
/* Public Methods                                                            */
/******************************************************************************/
/**
 * récupère l'imputation et la sous imputation budgétaire
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void bet_utils_get_budget_data (GtkWidget *widget,
								gint struct_type,
								gpointer *value)
{
    const gchar *string;
    gchar **tab_char;
    gint budgetary_number;
    gint sub_budgetary_number;

    string = gtk_combofix_get_text (GTK_COMBOFIX (widget));
    if (string && strlen (string) > 0)
    {
        tab_char = g_strsplit (string, " : ", 2);
        budgetary_number = gsb_data_budget_get_number_by_name (tab_char[0], FALSE, 0);

        if (tab_char[1] && strlen (tab_char[1]))
            sub_budgetary_number = gsb_data_budget_get_sub_budget_number_by_name (
														budgetary_number, tab_char[1], FALSE);
        else
            sub_budgetary_number = 0;

        g_strfreev (tab_char);
    }
    else
    {
        budgetary_number = 0;
        sub_budgetary_number = 0;
    }

    if (struct_type == 0)
    {
        FuturData *sd = (FuturData *) value;

        sd->budgetary_number = budgetary_number;
        sd->sub_budgetary_number = sub_budgetary_number;
    }
    else if (struct_type == 1)
    {
        TransfertData *sd = (TransfertData *) value;

        sd->main_budgetary_number = budgetary_number;
        sd->main_sub_budgetary_number = sub_budgetary_number;
    }
}

/**
 * récupère la catégorie et la sous catégorie
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void bet_utils_get_category_data (GtkWidget *widget,
								  gint struct_type,
								  gpointer *value)
{
    const gchar *string;
    gchar **tab_char;
    gint category_number = 0;
    gint sub_category_number = 0;

    string = gtk_combofix_get_text (GTK_COMBOFIX (widget));

    if (struct_type == 0)
    {
        FuturData *sd = (FuturData *) value;

        if (string && strlen (string) > 0)
        {
            tab_char = g_strsplit (string, " : ", 2);
            if (my_strcasecmp (tab_char[0], _("Transfer")) == 0)
            {
                sd->is_transfert = TRUE;
                sd->account_transfert =  gsb_data_account_get_no_account_by_name (tab_char[1]);
            }
            else
            {
                category_number = gsb_data_category_get_number_by_name (
                                tab_char[0], FALSE, 0);

                if (tab_char[1] && strlen (tab_char[1]))
                    sub_category_number = gsb_data_category_get_sub_category_number_by_name (
                                								category_number, tab_char[1], FALSE);
                else
                    sub_category_number = 0;
            }

            g_strfreev (tab_char);
        }
        else
        {
            category_number = 0;
            sub_category_number = 0;
        }

        sd->category_number = category_number;
        sd->sub_category_number = sub_category_number;
    }
    else if (struct_type == 1)
    {
        TransfertData *sd = (TransfertData *) value;

        if (string && strlen (string) > 0)
        {
            tab_char = g_strsplit (string, " : ", 2);
            category_number = gsb_data_category_get_number_by_name ( tab_char[0], FALSE, 0);

            if (tab_char[1] && strlen (tab_char[1]))
                sub_category_number = gsb_data_category_get_sub_category_number_by_name (
                            								category_number, tab_char[1], FALSE);
            else
                sub_category_number = 0;

            g_strfreev (tab_char);
        }
        else
        {
            category_number = 0;
            sub_category_number = 0;
        }

        sd->main_category_number = category_number;
        sd->main_sub_category_number = sub_category_number;
    }
    else if (struct_type == 2)
    {
        /* on est toujours avec une struture transfert mais on récupère les éléments pour le compte carte */
        TransfertData *sd = (TransfertData *) value;

        if (string && strlen (string) > 0)
        {
            tab_char = g_strsplit (string, " : ", 2);
            category_number = gsb_data_category_get_number_by_name ( tab_char[0], FALSE, 0);

            if (tab_char[1] && strlen (tab_char[1]))
                sub_category_number = gsb_data_category_get_sub_category_number_by_name (
                            								category_number, tab_char[1], FALSE);
            else
                sub_category_number = 0;

            g_strfreev (tab_char);
        }
        else
        {
            category_number = 0;
            sub_category_number = 0;
        }

        sd->card_category_number = category_number;
        sd->card_sub_category_number = sub_category_number;
    }
}

/**
 * récupère le tiers
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
void bet_utils_get_payee_data (GtkWidget *widget,
							   gint struct_type,
							   gpointer *value)
{
    const gchar *string;
    TransfertData *sd = (TransfertData *) value;

    string = gtk_combofix_get_text (GTK_COMBOFIX (widget));
    if (string && strlen (string) > 0)
    {
        if (struct_type == 1)
            sd->main_payee_number = gsb_data_payee_get_number_by_name (string, FALSE);
        else
            sd->card_payee_number = gsb_data_payee_get_number_by_name (string, FALSE);
    }
    else
    {
        if (struct_type == 1)
            sd->main_payee_number = 0;
        else
            sd->card_payee_number = 0;
    }
}

/**
 * récupère le moyen de payement
 *
 * \param
 * \param
 *
 * \return
 **/
void bet_utils_get_payment_data (GtkWidget *widget,
								 gint struct_type,
								 gpointer *value)
{
    gint payment_number;
    TransfertData *std = (TransfertData *) value;

    payment_number = gsb_payment_method_get_selected_number (widget);
    if (payment_number > 0)
    {
		if (struct_type == 1)
        	std->main_payment_number = payment_number;
		else
			std->card_payment_number = payment_number;
    }
    else
    {
		if (struct_type == 1)
        	std->main_payment_number = gsb_data_account_get_default_debit (std->main_account_number);
		else
			std->card_payment_number = gsb_data_account_get_default_credit (std->card_account_number);
    }
}

/* BET_FUTURE */
/******************************************************************************/
/* Private Methods                                                            */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *bet_future_create_dialog (gint account_number)
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *table;

    /* Create the dialog */
    dialog = gtk_dialog_new_with_buttons (_("Enter a budget line"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_DIALOG_MODAL,
										  "gtk-cancel", GTK_RESPONSE_CANCEL,
										  "gtk-ok", GTK_RESPONSE_OK,
										  NULL);

    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
    gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)), vbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), BOX_BORDER_WIDTH);

    /* next we fill the bet_form */
    table = gtk_grid_new ();
    gtk_grid_set_column_spacing (GTK_GRID (table), MARGIN_BOX);
    gtk_widget_show (table);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, MARGIN_BOX);

    bet_form_create_scheduler_part (dialog, table);
    bet_form_create_current_form (dialog, table, account_number);

	gtk_widget_show (vbox);

    return dialog;
}

/**
 * initialise les données du formulaire
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean bet_future_set_form_data_from_line (gint account_number,
													gint number)
{
    GtkWidget *widget;
    GHashTable *future_list;
    gchar *key;
    const gchar *tmp_str;
    FuturData *scheduled;

    if (account_number == 0)
	{
		gchar *tmp_str1;
		tmp_str1 = utils_str_itoa (number);
        key = g_strconcat ("0:", tmp_str1, NULL);
		g_free(tmp_str1);
	}
    else
	{
		gchar *tmp_str1;
		gchar *tmp_str2;
		tmp_str1 = utils_str_itoa (account_number);
		tmp_str2 = utils_str_itoa (number);
        key = g_strconcat (tmp_str1, ":", tmp_str2, NULL);
		g_free(tmp_str1);
		g_free(tmp_str2);
	}

    future_list = bet_data_future_get_list ();

    scheduled = g_hash_table_lookup (future_list, key);
    if (scheduled == NULL)
        return FALSE;

    /* On traite les données de la planification */
    widget = bet_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_BUTTON);
    gsb_combo_box_set_index (widget, scheduled->frequency);
    gtk_widget_set_sensitive (widget, FALSE);

    if (scheduled->frequency > 0)
    {
        if (scheduled->limit_date && g_date_valid (scheduled->limit_date))
        {
            widget = bet_form_scheduler_get_element_widget (SCHEDULED_FORM_LIMIT_DATE);
            gsb_form_widget_set_empty (widget, FALSE);
            gsb_calendar_entry_set_date (widget, scheduled->limit_date);
            gtk_widget_set_sensitive (widget, FALSE);
        }

        if (scheduled->user_entry > 0)
        {
            widget = bet_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_ENTRY);
            gsb_form_widget_set_empty (widget, FALSE);
            gtk_entry_set_text (GTK_ENTRY (widget), utils_str_itoa (scheduled->user_entry));
            gtk_widget_set_sensitive (widget, FALSE);

            widget = bet_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_BUTTON);
            gsb_combo_box_set_index (widget, scheduled->user_interval);
            gtk_widget_set_sensitive (widget, FALSE);
        }
    }

    /* On traite les données de transaction */
    widget = bet_form_widget_get_widget (TRANSACTION_FORM_DATE);
    gsb_calendar_entry_set_date (widget, scheduled->date);
    gsb_form_widget_set_empty (widget, FALSE);

    if (scheduled->fyear_number > 0)
    {
        widget = bet_form_widget_get_widget (TRANSACTION_FORM_EXERCICE);
        gsb_fyear_set_combobox_history (widget, scheduled->fyear_number);
    }

    widget = bet_form_widget_get_widget (TRANSACTION_FORM_PARTY);
    tmp_str = gsb_data_payee_get_name (scheduled->party_number, TRUE);
    if (tmp_str && strlen (tmp_str))
    {
        gsb_form_widget_set_empty (widget, FALSE);
        gtk_combofix_set_text (GTK_COMBOFIX (widget), tmp_str);
        gtk_editable_set_position (GTK_EDITABLE (gtk_combofix_get_entry (GTK_COMBOFIX (widget))), 0);
    }

    if (scheduled->amount.mantissa < 0)
    {
        widget = bet_form_widget_get_widget (TRANSACTION_FORM_DEBIT);
        gtk_entry_set_text (GTK_ENTRY (widget), utils_real_get_string (gsb_real_opposite (scheduled->amount)));
    }
    else
    {
        widget = bet_form_widget_get_widget (TRANSACTION_FORM_CREDIT);
        gtk_entry_set_text (GTK_ENTRY (widget), utils_real_get_string (scheduled->amount));
    }
    gsb_form_widget_set_empty (widget, FALSE);

    widget = bet_form_widget_get_widget (TRANSACTION_FORM_TYPE);
    gsb_payment_method_set_combobox_history (widget, scheduled->payment_number, TRUE);

    if (scheduled->category_number > 0)
    {
        widget = bet_form_widget_get_widget (TRANSACTION_FORM_CATEGORY);
        gsb_form_widget_set_empty (widget, FALSE);
        gtk_combofix_set_text (GTK_COMBOFIX (widget),
							   gsb_data_category_get_name (scheduled->category_number,
														   scheduled->sub_category_number, NULL));
    }
    else if (scheduled->is_transfert)
    {
        gchar *tmp_str_2;

        widget = bet_form_widget_get_widget (TRANSACTION_FORM_CATEGORY);
        gsb_form_widget_set_empty (widget, FALSE);
        tmp_str_2 = g_strconcat (_("Transfer"),
								 " : ",
								 gsb_data_account_get_name (scheduled->account_transfert),
								 NULL);

        gtk_combofix_set_text (GTK_COMBOFIX (widget), tmp_str_2);
        g_free (tmp_str_2);
    }

    if (scheduled->budgetary_number > 0)
    {
        widget = bet_form_widget_get_widget (TRANSACTION_FORM_BUDGET);
        gsb_form_widget_set_empty (widget, FALSE);
        gtk_combofix_set_text (GTK_COMBOFIX (widget),
							   gsb_data_budget_get_name (scheduled->budgetary_number,
														 scheduled->sub_budgetary_number, NULL));
    }

    if (scheduled->notes && strlen (scheduled->notes) > 0)
    {
        widget = bet_form_widget_get_widget (TRANSACTION_FORM_NOTES);
        gsb_form_widget_set_empty (widget, FALSE);
        gtk_entry_set_text (GTK_ENTRY (widget), scheduled->notes);
    }

    return TRUE;
}

/**
 * récupère les données du formulaire
 *
 * \param
 *
 * \return
 **/
static gboolean bet_future_take_data_from_form (FuturData *scheduled)
{
    GtkWidget *widget;

    /* données liées au compte */
    scheduled->account_number = gsb_gui_navigation_get_current_account ();

    /* On traite les données de la planification */
    widget = bet_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_BUTTON);
    scheduled->frequency = gsb_combo_box_get_index (widget);

    switch (scheduled->frequency)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
		{
			widget = bet_form_scheduler_get_element_widget (SCHEDULED_FORM_LIMIT_DATE);
            if (gsb_form_widget_check_empty (widget) == FALSE)
			{
				GrisbiWinEtat *w_etat;

				w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
				if (w_etat->form_date_force_prev_year)
				{
					w_etat->form_date_force_prev_year = FALSE;
					scheduled->limit_date = gsb_calendar_entry_get_date (widget);
					w_etat->form_date_force_prev_year = TRUE;
				}
				else
					scheduled->limit_date = gsb_calendar_entry_get_date (widget);
			}
            else
                scheduled->limit_date = NULL;
            break;
		}
        case 6:
            if (gsb_form_widget_check_empty (widget) == FALSE)
			{
				GrisbiWinEtat *w_etat;

				w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
				if (w_etat->form_date_force_prev_year)
				{
					w_etat->form_date_force_prev_year = FALSE;
					scheduled->limit_date = gsb_calendar_entry_get_date (widget);
					w_etat->form_date_force_prev_year = TRUE;
				}
				else
					scheduled->limit_date = gsb_calendar_entry_get_date (widget);
			}
            else
                scheduled->limit_date = NULL;

			widget = bet_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_ENTRY);
            if (gsb_form_widget_check_empty (widget) == FALSE)
                scheduled->user_entry = utils_str_atoi (gtk_entry_get_text (GTK_ENTRY (widget)));
            if (scheduled->user_entry)
            {
                widget = bet_form_scheduler_get_element_widget (SCHEDULED_FORM_FREQUENCY_USER_BUTTON);
                scheduled->user_interval = gsb_combo_box_get_index (widget);
            }
            else
                return FALSE;
    }

    /* On traite les données de transaction */
    widget = bet_form_widget_get_widget (TRANSACTION_FORM_DATE);
    if (gsb_form_widget_check_empty (widget) == FALSE)
    {
        GDate *date_tomorrow;

        date_tomorrow = gsb_date_tomorrow ();
        scheduled->date = gsb_calendar_entry_get_date (widget);
        if (scheduled->date == NULL
			||
			g_date_compare (scheduled->date, date_tomorrow) < 0)
        {
            g_date_free (date_tomorrow);
            return FALSE;
        }
        g_date_free (date_tomorrow);
    }
    else
        return FALSE;

    widget = bet_form_widget_get_widget (TRANSACTION_FORM_EXERCICE);
	scheduled->fyear_number = gsb_fyear_get_fyear_from_combobox (widget,
					scheduled->date);
    widget = bet_form_widget_get_widget (TRANSACTION_FORM_PARTY);
    if (gsb_form_widget_check_empty (widget) == FALSE)
        scheduled->party_number = gsb_data_payee_get_number_by_name (
                        gtk_combofix_get_text (GTK_COMBOFIX (widget)), TRUE);
    else
        scheduled->party_number = 0;

    widget = bet_form_widget_get_widget (TRANSACTION_FORM_DEBIT);
    if (gsb_form_widget_check_empty (widget) == FALSE)
    {
        gsb_form_check_auto_separator (widget);
        scheduled->amount = gsb_real_opposite (utils_real_get_calculate_entry (widget));
    }
    else
    {
        widget = bet_form_widget_get_widget (TRANSACTION_FORM_CREDIT);
        if (gsb_form_widget_check_empty (widget) == FALSE)
        {
            gsb_form_check_auto_separator (widget);
            scheduled->amount = utils_real_get_calculate_entry (widget);
        }
        else
            return FALSE;
    }

    widget = bet_form_widget_get_widget (TRANSACTION_FORM_TYPE);
    scheduled->payment_number = gsb_payment_method_get_selected_number (widget);

    widget = bet_form_widget_get_widget (TRANSACTION_FORM_CATEGORY);
    if (gsb_form_widget_check_empty(widget) == FALSE)
        bet_utils_get_category_data (widget, 0, (gpointer) scheduled);
    else
    {
        scheduled->category_number = 0;
        scheduled->sub_category_number = 0;
    }

    widget = bet_form_widget_get_widget (TRANSACTION_FORM_BUDGET);
    if (gsb_form_widget_check_empty(widget) == FALSE)
        bet_utils_get_budget_data (widget, 0, (gpointer) scheduled);
    else
    {
        scheduled->budgetary_number = 0;
        scheduled->sub_budgetary_number = 0;
    }

    widget = bet_form_widget_get_widget (TRANSACTION_FORM_NOTES);
    if (gsb_form_widget_check_empty(widget) == FALSE)
        scheduled->notes = g_strdup (gtk_entry_get_text (GTK_ENTRY (widget)));
    else
        scheduled->notes = NULL;

    return TRUE;
}

/******************************************************************************/
/* Public Methods                                                             */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
void bet_future_initialise_dialog (gboolean cleanup)
{

    if (bet_futur_dialog != NULL)
    {
        gtk_widget_destroy (bet_futur_dialog);
        bet_futur_dialog = NULL;
    }

    if (bet_transfert_dialog != NULL)
    {
        gtk_widget_destroy (bet_transfert_dialog);
        bet_transfert_dialog = NULL;
    }

	if (bfo_struct)
	{
		g_free (bfo_struct);
		bfo_struct = NULL;
	}

	if (cleanup)
	{
		bet_form_set_organization ();
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
gboolean bet_future_new_line_dialog (GtkTreeModel *tab_model,
									 gchar *str_date)
{
    GtkWidget *widget;
    gchar *tmp_str;
    GDate *date;
    GDate *date_jour;
    gint result;
    gint account_number;

    account_number = gsb_gui_navigation_get_current_account ();
    if (account_number == -1)
        return FALSE;

    if (bet_futur_dialog == NULL)
    {
        bet_futur_dialog = bet_future_create_dialog (account_number);
    }
    else
    {
        bet_form_clean (account_number);
        gtk_widget_show (bet_futur_dialog);
    }

    /* init data */
    widget = bet_form_widget_get_widget (TRANSACTION_FORM_DATE);
    date = gsb_parse_date_string (str_date);
    date_jour = gdate_today ();

    if (g_date_valid (date))
    {
        if (g_date_compare (date_jour, date) >= 0)
        {
            g_date_free (date);
            g_date_add_days (date_jour, 1);
            date = date_jour;
        }
    }
    else
    {
        g_date_add_days (date_jour, 1);
        date = date_jour;
    }

    gsb_form_widget_set_empty (widget, FALSE);
    gsb_calendar_entry_set_date (widget, date);

    gtk_dialog_set_response_sensitive (GTK_DIALOG (bet_futur_dialog), GTK_RESPONSE_OK, FALSE);

dialog_return:
	result = gtk_dialog_run (GTK_DIALOG (bet_futur_dialog));

    if (result == GTK_RESPONSE_OK)
    {
        FuturData *scheduled;

        scheduled = bet_data_future_struct_init ();

        if (!scheduled)
        {
            dialogue_error_memory ();
            gtk_widget_hide (bet_futur_dialog);
            return FALSE;
        }

        if (bet_future_take_data_from_form (scheduled) == FALSE)
        {
            tmp_str = g_strdup (_("Error: the frequency defined by the user or the amount is "
                                 "not specified or the date is invalid."));
            dialogue_warning_hint (tmp_str, _("One field is not filled in"));
            g_free (tmp_str);
			bet_data_future_struct_free(scheduled);
            goto dialog_return;
        }
        else
            bet_data_future_add_lines (scheduled);

        gsb_data_account_set_bet_maj (account_number, BET_MAJ_ESTIMATE);
        bet_data_update_bet_module (account_number, GSB_ESTIMATE_PAGE);
    }

    gtk_widget_hide (bet_futur_dialog);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean bet_future_modify_line (gint account_number,
								 gint number,
								 gint mother_row)
{
    gchar *tmp_str;
    gint result;

    if (bet_futur_dialog == NULL)
    {
        bet_futur_dialog = bet_future_create_dialog (account_number);
    }
    else
    {
        bet_form_clean (account_number);
        gtk_widget_show (bet_futur_dialog);
    }

     /* init data */
    bet_future_set_form_data_from_line (account_number, number);

dialog_return:
    result = gtk_dialog_run (GTK_DIALOG (bet_futur_dialog));

    if (result == GTK_RESPONSE_OK)
    {
        FuturData *scheduled;

        scheduled = bet_data_future_struct_init ();

        if (!scheduled)
        {
            dialogue_error_memory ();
            gtk_widget_hide (bet_futur_dialog);
            return FALSE;
        }

        if (bet_future_take_data_from_form (scheduled) == FALSE)
        {
            tmp_str = g_strdup (_("Error: the frequency defined by the user or the amount is "
								  "not specified or the date is invalid."));
            dialogue_warning_hint (tmp_str, _("One field is not filled in"));
            g_free (tmp_str);
            goto dialog_return;
        }
        else
        {
            scheduled->number = number;
            scheduled->mother_row = mother_row;
            bet_data_future_modify_lines (scheduled);
        }

        gsb_data_account_set_bet_maj (account_number, BET_MAJ_ESTIMATE);
        bet_data_update_bet_module (account_number, GSB_ESTIMATE_PAGE);
    }

    gtk_widget_hide (bet_futur_dialog);

    return FALSE;
}

/* BET_TRANSFERT */
/******************************************************************************/
/* Private Methods                                                            */
/******************************************************************************/
/******************************************************************************/
/* Public Methods                                                             */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean bet_transfert_new_line_dialog (GtkTreeModel *tab_model,
										gchar *str_date)
{
	GtkWidget *transfer_dialog;
    GDate *date_debit;
    gint account_number;
    gint result;

	devel_debug (str_date);
    account_number = gsb_gui_navigation_get_current_account ();
    if (account_number == -1)
        return FALSE;

    /* Create the dialog */
    transfer_dialog = GTK_WIDGET (widget_transfer_new (account_number, TRUE));
    if (transfer_dialog == NULL)
    {
        dialogue_warning (_("You must have at least one account with deferred debit card."));
        return FALSE;
    }

    /* init data */
	date_debit = gsb_parse_date_string (str_date);
	if (g_date_valid (date_debit) == FALSE)
		date_debit = gdate_today ();

	widget_transfer_set_date_debit (transfer_dialog, date_debit);

dialog_return:
    result = gtk_dialog_run (GTK_DIALOG (transfer_dialog));

    if (result == GTK_RESPONSE_OK)
    {
        gchar *tmp_str;
        TransfertData *transfert;

        transfert = widget_transfer_take_data (transfer_dialog, account_number, 0);
        if (!transfert)
        {
            tmp_str = g_strdup (_("Error: a date is not present or a field is in error."));
            dialogue_warning_hint (tmp_str, _("One field is not filled in"));
            g_free (tmp_str);
            goto dialog_return;
        }
        else
            bet_data_transfert_add_line (transfert);

        gsb_data_account_set_bet_maj (account_number, BET_MAJ_ESTIMATE);
        bet_data_update_bet_module (account_number, GSB_ESTIMATE_PAGE);
    }

    gtk_widget_destroy (transfer_dialog);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GsbReal gsb_transfert_get_str_amount (GsbReal amount,
									  gint account_currency,
									  gint replace_currency,
									  gint floating_point)
{
    GsbReal tmp_real = null_real;
    gint link_number;

    if ((link_number = gsb_data_currency_link_search (account_currency, replace_currency)))
    {
        if (gsb_data_currency_link_get_first_currency (link_number) == replace_currency)
            tmp_real = gsb_real_mul (amount,
									 gsb_data_currency_link_get_change_rate (link_number));
        else
            tmp_real = gsb_real_div (amount,
									 gsb_data_currency_link_get_change_rate (link_number));
    }
    else if (account_currency > 0 && replace_currency > 0)
    {
        gchar *tmp_str;

        tmp_str = g_strdup (_("Error: is missing one or more links between currencies.\n"
							  "You need to fix it and start over."));
        dialogue_error (tmp_str);

        g_free (tmp_str);
    }

    return tmp_real;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void bet_transfert_modify_line (gint account_number,
								gint number)
{
	GtkWidget *transfer_dialog;
	gint result;

	devel_debug (NULL);
	transfer_dialog = GTK_WIDGET (widget_transfer_new (account_number, FALSE));

	if (transfer_dialog == NULL)
	{
		gchar *tmp_str;

		tmp_str = g_strdup (_("There is an inconsistency in the balance of "
							  "a deferred debit account.\n"
							  "You should delete it and create another"));

		dialogue_error_hint (tmp_str, _("Error of configuration"));

		return;
    }

    /* init data */
    widget_transfer_fill_data_from_line (transfer_dialog, account_number, number);

dialog_return:
    result = gtk_dialog_run (GTK_DIALOG (transfer_dialog));
    if (result == GTK_RESPONSE_OK)
    {
        gchar *tmp_str;
        TransfertData *std;

		std = widget_transfer_take_data (transfer_dialog, account_number, number);
        if (!std)
        {
            tmp_str = g_strdup (_("Error: a date is not present or a field is in error."));
            dialogue_warning_hint (tmp_str, _("One field is not filled in"));
            g_free (tmp_str);
			goto dialog_return;
        }
        else
        {
            bet_data_transfert_modify_line (std);
        }

        gsb_data_account_set_bet_maj (account_number, BET_MAJ_ESTIMATE);
        bet_data_update_bet_module (account_number, GSB_ESTIMATE_PAGE);
    }
	else if (result == GTK_RESPONSE_REJECT)
	{
		gchar *msg;
		const gchar *tmp_str;
		TransfertData *std;

		std = bet_data_transfert_get_struct_from_number (number);
		if (std->type == 0)
			tmp_str = gsb_data_account_get_name ( std->card_account_number );
		else
			tmp_str = gsb_data_partial_balance_get_name ( std->card_account_number );

		msg = g_strdup_printf (_("Warning: You are about to delete the deferred debit card: \"%s\"\n"
								 "Are you sure?"),
							   tmp_str);
        if (dialogue_yes_no (msg, _("Confirmation of deletion of a deferred debit card"), GTK_RESPONSE_CANCEL))
			bet_data_transfert_remove_line (account_number, number);
		g_free (msg);
	}
    gtk_widget_destroy (transfer_dialog);
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
