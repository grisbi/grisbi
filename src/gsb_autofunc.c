/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cedric Auger (cedric@grisbi.org)	          */
/*			2003-2009 Benjamin Drieu (bdrieu@april.org)	                      */
/* 			https://www.grisbi.org				                              */
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
 * \file gsb_autofunc.c
 * this file groups some widget functions which update value automatically
 * when the widget change
 * a change in the widget call a given function of kind gsb_data_..._set_... (number, value)
 * use to set automatically the content in the grisbi structures
 * (for changes only on variables, see gsb_automem.c)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"

/*START_INCLUDE*/
#include "gsb_autofunc.h"
#include "gsb_calendar_entry.h"
#include "gsb_combo_box.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_bank.h"
#include "gsb_data_category.h"
#include "gsb_data_fyear.h"
#include "gsb_data_reconcile.h"
#include "gsb_file.h"
#include "utils_editables.h"
#include "gsb_real.h"
#include "traitement_variables.h"
#include "utils_real.h"
#include "utils_str.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/**
 * called when something change in an button of a gsb_editable_checkbutton
 * by gsb_autofunc_entry_new
 *
 * \param button The reference GtkCheckButton
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_checkbutton_changed (GtkWidget *button,
												  gboolean default_func (gint,
																		 gboolean))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !button)
		return FALSE;

    number_for_func = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (button), "number_for_func"));
    default_func (number_for_func, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)));

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

    return FALSE;
}

/**
 * called when the place change in the autofunc combobox
 *
 * \param combobox The reference Combobox
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_combobox_changed (GtkWidget *combobox,
											   gboolean default_func (gint,
																	  gint))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !combobox)
		return FALSE;

    number_for_func = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (combobox), "number_for_func"));
    default_func (number_for_func, gsb_combo_box_get_index (combobox));

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

	return FALSE;
}

/**
 * called when the place change in the autofunc currency
 *
 * \param combobox The reference Combobox
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_currency_changed (GtkWidget *combobox,
											   gboolean default_func (gint,
																	  gint))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !combobox)
		return FALSE;

    number_for_func = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (combobox), "number_for_func"));
    default_func (number_for_func, gsb_currency_get_currency_from_combobox (combobox));

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

    return FALSE;
}

/**
 * called when something change in an entry of a gsb_editable_date
 * by gsb_autofunc_date_new
 *
 * \param entry The reference GtkEntry
 * \param default_func the function to call to change the date in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_date_changed (GtkWidget *entry,
										   gboolean default_func (gint,
																  const GDate *))
{
    gint number_for_func;
    GDate *date;

    /* just to be sure... */
    if (!default_func || !entry)
		return FALSE;

    number_for_func = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry), "number_for_func"));
    date = gsb_calendar_entry_get_date (entry);

    default_func (number_for_func, date);

    if (date)
		g_date_free (date);

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

    return FALSE;
}

/**
 * called when something change in an entry of a gsb_editable_text
 * by gsb_autofunc_entry_new
 *
 * \param entry The reference GtkEntry
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_entry_changed (GtkWidget *entry,
											gboolean default_func (gint,
																   const gchar *))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !entry)
		return FALSE;

    number_for_func = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry), "number_for_func"));
    default_func (number_for_func, gtk_entry_get_text (GTK_ENTRY (entry)));

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

    return FALSE;
}

/**
 * called when something change in an entry of a gsb_editable_int
 * by gsb_autofunc_int_new
 *
 * \param entry The reference GtkEntry
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_int_changed (GtkWidget *entry,
										  gboolean default_func (gint,
																 gint))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !entry)
		return FALSE;

    number_for_func = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry), "number_for_func"));
    default_func (number_for_func, utils_str_atoi (gtk_entry_get_text (GTK_ENTRY (entry))));

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

    return FALSE;
}

/**
 * called when something change in an entry of a gsb_autofunc_real
 * by gsb_autofunc_real_new
 *
 * \param entry The reference GtkEntry
 * \param default_func the function to call to change the date in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_real_changed (GtkWidget *entry,
										   gboolean default_func (gint,
																  GsbReal))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !entry)
		return FALSE;

    number_for_func = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry), "number_for_func"));
    default_func (number_for_func, utils_real_get_from_string (gtk_entry_get_text (GTK_ENTRY (entry))));

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

	return FALSE;
}

/**
 * called when something change in an spin_button of a autofunc_spin
 * 	by gsb_autofunc_spin_new
 *
 * \param spin_button The reference GtkSpinButton
 * \param default_func the function to call to change the value in memory
 *
 * \return FALSE
 */
static gboolean gsb_autofunc_spin_changed (GtkWidget *spin_button,
										   gboolean default_func (gint,
																  gint))
{
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !spin_button)
		return FALSE;

    number_for_func = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (spin_button), "number_for_func"));
    default_func (number_for_func, gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin_button)));

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

	return FALSE;
}

/**
 * Set a string to the value of an GtkTextView
 *
 * \param buffer The reference GtkTextBuffer
 * \param dummy Handler parameter.  Not used.
 */
static gboolean gsb_autofunc_textview_changed (GtkTextBuffer *buffer,
											   gboolean default_func (gint,
																	  const gchar *))
{
    GtkTextIter start, end;
    gint number_for_func;

    /* just to be sure... */
    if (!default_func || !buffer)
		return FALSE;

    number_for_func = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (buffer), "number_for_func"));

    gtk_text_buffer_get_iter_at_offset (buffer, &start, 0);
    gtk_text_buffer_get_iter_at_offset (buffer, &end, -1);

    default_func (number_for_func, gtk_text_buffer_get_text (buffer, &start, &end, 0));

    /* Mark file as modified */
    gsb_file_set_modified (TRUE);

    return FALSE;
}

/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/* CHECK_BUTTON */
/*
 * creates a new checkbox associated to a value in a grisbi structure
 * for each change, will call the corresponding given function : gsb_data_... (number, gboolean)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 gboolean yes/no)
 * ex : gsb_data_fyear_set_form_show (fyear_number, showed_in_form)
 *
 * \param label the text associated to the checkbox
 * \param value a boolean for the state of the checkbox
 * \param hook an optional function to execute as a handler if the
 * 	button is modified.
 * 	hook should be :
 * 		gboolean hook (GtkWidget *button,
 * 				gpointer data)
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func (number, gboolean)) or NULL
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 *
 * \return a new GtkCheckButton
 * */
GtkWidget *gsb_autofunc_checkbutton_new (const gchar *label,
										 gboolean value,
										 GCallback hook,
										 gpointer data,
										 GCallback default_func,
										 gint number_for_func)
{
    GtkWidget *button;

    /* first, create and set the button */
    button = gtk_check_button_new_with_mnemonic (label);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), value);

    /* set the default func :
     * the func will be send to gsb_autofunc_checkbutton_changed by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (button), "number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (button),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(button),
																	 "toggled",
																	 ((GCallback) gsb_autofunc_checkbutton_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (button),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(button),
																	 "toggled",
																	 ((GCallback) hook),
																	 data)));
    return button;
}

/*
 * creates a new checkbox from ui file associated to a value in a grisbi structure
 * for each change, will call the corresponding given function : gsb_data_... (number, gboolean)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 gboolean yes/no)
 * ex : gsb_data_fyear_set_form_show (fyear_number, showed_in_form)
 *
 * \param label the text associated to the checkbox
 * \param value a boolean for the state of the checkbox
 * \param hook an optional function to execute as a handler if the
 * 	button is modified.
 * 	hook should be :
 * 		gboolean hook (GtkWidget *button,
 * 				gpointer data)
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func (number, gboolean)) or NULL
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 *
 * \return a new GtkCheckButton
 * */
void gsb_autofunc_checkbutton_new_from_ui (GtkWidget *button,
										   gboolean value,
										   GCallback hook,
										   gpointer data,
										   GCallback default_func,
										   gint number_for_func)
{
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), value);

    /* set the default func :
     * the func will be send to gsb_autofunc_checkbutton_changed by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (button), "number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (button),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(button),
																	 "toggled",
																	 ((GCallback) gsb_autofunc_checkbutton_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (button),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(button),
																	 "toggled",
																	 ((GCallback) hook),
																	 data)));
}

/**
 * set the value in a gsb_editable_checkbutton
 * a value is in 2 parts :
 * 	a boolean, so value TRUE or FALSE
 * 	a number, which is used when there is a change in that button (see gsb_autofunc_checkbutton_new)
 *
 * \param button
 * \param value a gboolean
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_checkbutton_set_value (GtkWidget *button,
										 gboolean value,
										 gint number_for_func)
{
    gulong changed;
    gulong changed_hook;

    /* Block everything */
    changed = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (button), "changed"));
    if (changed > 0)
        g_signal_handler_block (G_OBJECT (button), changed);

    changed_hook = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (button), "changed-hook"));
    if (changed_hook > 0)
        g_signal_handler_block (G_OBJECT (button), changed_hook);

    /* Fill in value */
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), value);
    g_object_set_data (G_OBJECT (button), "number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if (changed > 0)
        g_signal_handler_unblock (G_OBJECT (button), changed);

    if (changed_hook > 0)
        g_signal_handler_unblock (G_OBJECT (button), changed_hook);
}

/* COMBO_BOX */
/*
 * creates a new Combobox which will modify the value according to the index
 * but made for values in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... (number, index)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 gint index)
 * ex : gsb_data_account_set_kind (account_number, account_kind)
 *
 * basically, that combobox is created with gsb_combo_box_new_with_index_from_list, so can use that functions to get the index
 * 	if necessary
 *
 * \param list a g_slist to create the combobox (succession of text and number, see gsb_combo_box_new_with_index_from_list)
 * \param index the index to place the combobox
 * \param hook an optional function to execute as a handler if the
 * 	combobox changed.
 * 	hook should be :
 * 		gboolean hook (GtkWidget *combobox,
 * 				gpointer data)
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func (number, number)) or NULL
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 * 				that number can be changed with gsb_autofunc_combobox_set_index
 *
 * \return a new GtkComboBox
 * */
GtkWidget *gsb_autofunc_combobox_new (GSList *list,
									  gint index,
									  GCallback hook,
									  gpointer data,
									  GCallback default_func,
									  gint number_for_func)
{
    GtkWidget *combobox;

    /* create and fill the combobox */
    combobox = gsb_combo_box_new_with_index_from_list (list, NULL, NULL);

    gsb_combo_box_set_index (combobox, index);

    /* set the default func :
     * the func will be sent to gsb_autofunc_combobox_changed by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (combobox), "number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (combobox),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(combobox),
																	 "changed",
																	 G_CALLBACK (gsb_autofunc_combobox_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (combobox),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(combobox),
																	 "changed",
																	 G_CALLBACK (hook),
																	 data)));
    return combobox;
}

/**
 * set the value in a gsb_autofunc_combobox
 * a value is in 2 parts :
 * 	an index, which place the combobox on the good place
 * 	a number, which is used when there is a change in that combobox (see gsb_autofunc_combobox_new)
 *
 * \param combobox
 * \param index the index to place the combobox
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_combobox_set_index (GtkWidget *combobox,
									  gint index,
									  gint number_for_func)
{
    gulong changed;
    gulong changed_hook;

    /* Block everything */
    changed = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (combobox), "changed"));
    if (changed > 0)
        g_signal_handler_block (G_OBJECT (combobox), changed);

    changed_hook = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (combobox), "changed-hook"));
    if (changed_hook > 0)
        g_signal_handler_block (G_OBJECT(combobox), changed_hook);

    /* place the combobox */
    gsb_combo_box_set_index (combobox, index);

    g_object_set_data (G_OBJECT (combobox),
					   "number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if (changed > 0)
        g_signal_handler_unblock (G_OBJECT (combobox), changed);

    if (changed_hook > 0)
        g_signal_handler_unblock (G_OBJECT (combobox), changed_hook);
}

/* CURRENCY */
/*
 * This create a combobox of currencies, this is basically the same as gsb_autofunc_combobox_new
 * 	but work only with currencies
 *
 * for each change, will call the corresponding given function : gsb_data_... (number, currency_number)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 gint currency_number)
 * ex : gsb_data_account_set_currency (account_number, currency_number)
 *
 * basically, that combobox is created with gsb_currency_make_combobox, so can use that functions to get the index
 * 	if necessary
 *
 * \param set_name TRUE to show the name of the currencies in the combobox
 * \param currency_number the currency we want to show
 * \param hook an optional function to execute as a handler if the
 * 	combobox changed.
 * 	hook should be :
 * 		gboolean hook (GtkWidget *combobox,
 * 				gpointer data)
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func (number, currency_number)) or NULL
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 * 				that number can be changed with gsb_autofunc_currency_set_currency_number
 *
 * \return a new GtkComboBox
 * */
GtkWidget *gsb_autofunc_currency_new (gboolean set_name,
									  gint currency_number,
									  GCallback hook,
									  gpointer data,
									  GCallback default_func,
									  gint number_for_func)
{
    GtkWidget *combobox;

    /* create and fill the combobox */
    combobox = gsb_currency_make_combobox (set_name);

    if (combobox && currency_number)
        gsb_currency_set_combobox_history (combobox, currency_number);

    /* set the default func :
     * the func will be sent to gsb_autofunc_currency_changed by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (combobox), "number_for_func", GINT_TO_POINTER (number_for_func));

    if (default_func)
		g_object_set_data (G_OBJECT (combobox),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(combobox),
																	 "changed",
																	 G_CALLBACK (gsb_autofunc_currency_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (combobox),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(combobox),
																	 "changed",
																	 G_CALLBACK (hook),
																	 data)));
     return combobox;
}

/**
 * show the currency in a gsb_autofunc_currency
 * a value is in 2 parts :
 * 	an currency number, which place the combobox on the good place
 * 	a number, which is used when there is a change in that combobox (see gsb_autofunc_currency_new)
 *
 * \param combobox
 * \param currency_number the currency to place the combobox
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_currency_set_currency_number (GtkWidget *combobox,
												gint currency_number,
												gint number_for_func)
{
    gulong changed;
    gulong changed_hook;

    /* Block everything */
    changed = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (combobox), "changed"));
    if (changed > 0)
        g_signal_handler_block (G_OBJECT (combobox), changed);

    changed_hook = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (combobox), "changed-hook"));
    if (changed_hook > 0)
        g_signal_handler_block (G_OBJECT (combobox), changed_hook);

    /* place the combobox */
    gsb_currency_set_combobox_history (combobox, currency_number);

    g_object_set_data (G_OBJECT (combobox), "number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if (changed > 0)
        g_signal_handler_unblock (G_OBJECT (combobox), changed);

    if (changed_hook > 0)
        g_signal_handler_unblock (G_OBJECT (combobox), changed_hook);
}

/* DATE */
/*
 * creates a new GtkEntry to contain dates which will modify the value according to the entry
 * but made for values in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... (number, date)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 GDate *date)
 * ex : gsb_data_reconcile_set_init_date (reconcile_number, name)
 *
 * it uses the gsb_calendar, so every options of the gsb_calendar work here
 *
 * \param date a date to fill the entry or NULL
 * \param hook an optional function to execute as a handler if the
 * 	entry's contents are modified.
 * 	hook should be :
 * 		gboolean hook (GtkWidget *entry,
 * 				gpointer data)
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func a function to call when something change (function must be func (number, date)) or NULL
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 * 				that number can be changed with gsb_autofunc_entry_set_value
 *
 * \return a new GtkEntry
 * */
GtkWidget *gsb_autofunc_date_new (const GDate *date,
								  GCallback hook,
								  gpointer data,
								  GCallback default_func,
								  gint number_for_func)
{
    GtkWidget *entry;

    /* first, create and fill the entry */
    entry = gsb_calendar_entry_new (TRUE);

    gsb_calendar_entry_set_date (entry, date);

    /* set the default func :
     * the func will be send to gsb_editable_set_text by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (entry), "number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (entry),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(entry),
																	 "changed",
																	 G_CALLBACK (gsb_autofunc_date_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (entry),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(entry),
																	 "changed",
																	 G_CALLBACK (hook),
																	 data)));
    return entry;
}

/**
 * set the date in a gsb_editable_date
 * a value is in 2 parts :
 * 	a date, which be showed in the entry
 * 	a number, which is used when there is a change in that entry (see gsb_autofunc_date_new)
 *
 * \param entry
 * \param date a date to set in the entry
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_date_set (GtkWidget *entry,
							const GDate *date,
							gint number_for_func)
{
    gulong changed;
    gulong changed_hook;

    /* Block everything */
    changed = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (entry), "changed"));
    if (changed > 0)
        g_signal_handler_block (G_OBJECT (entry), changed);

    changed_hook = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (entry), "changed-hook"));
    if (changed_hook > 0)
        g_signal_handler_block (G_OBJECT (entry), changed_hook);

    /* Fill in value */
    gsb_calendar_entry_set_date (entry, date);

    g_object_set_data (G_OBJECT (entry), "number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if (changed > 0)
        g_signal_handler_unblock (G_OBJECT (entry), changed);

    if (changed_hook > 0)
        g_signal_handler_unblock (G_OBJECT (entry), changed_hook);
}

/* ENTRY */
/*
 * creates a new GtkEntry which will modify the value according to the entry
 * but made for values in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... (number, string content)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 gchar *string)
 * ex : gsb_data_account_set_name (account_number, name)
 *
 * \param value a string to fill the entry
 * \param hook an optional function to execute as a handler if the
 * 	entry's contents are modified.
 * 	hook should be :
 * 		gboolean hook (GtkWidget *entry,
 * 				gpointer data)
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func (number, string)) or NULL
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 * 				that number can be changed with gsb_autofunc_entry_set_value
 *
 * \return a new GtkEntry
 * */
GtkWidget *gsb_autofunc_entry_new (const gchar *value,
								   GCallback hook,
								   gpointer data,
								   GCallback default_func,
								   gint number_for_func)
{
    GtkWidget *entry;

    /* first, create and fill the entry */
    entry = gtk_entry_new ();

    if (value)
		gtk_entry_set_text (GTK_ENTRY(entry), value);

    /* set the default func :
     * the func will be send to gsb_editable_set_text by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (entry),
					   "number_for_func",
					   GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (entry),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(entry),
																	 "changed",
																	 G_CALLBACK (gsb_autofunc_entry_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (entry),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(entry),
																	 "changed",
																	 G_CALLBACK (hook), data)));
    return entry;
}

void gsb_autofunc_entry_new_from_ui (GtkWidget *entry,
									 GCallback hook,
									 gpointer data,
									 GCallback default_func,
									 gint number_for_func)
{
    /* set the default func :
     * the func will be send to gsb_editable_set_text by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (entry),
					   "number_for_func",
					   GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (entry),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(entry),
																	 "changed",
																	 G_CALLBACK (gsb_autofunc_entry_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (entry),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(entry),
																	 "changed",
																	 G_CALLBACK (hook), data)));
}

/**
 * set the value in a gsb_editable_entry
 * a value is in 2 parts :
 * 	a string, which be showed in the entry
 * 	a number, which is used when there is a change in that entry (see gsb_autofunc_entry_new)
 *
 * \param entry
 * \param value a string to set in the entry
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_entry_set_value (GtkWidget *entry,
                                   const gchar *value,
                                   gint number_for_func)
{
    gulong changed;
    gulong changed_hook;

    /* Block everything */
    changed = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (entry), "changed"));
    if (changed > 0)
        g_signal_handler_block (G_OBJECT (entry), changed);

    changed_hook = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (entry), "changed-hook"));
    if (changed_hook > 0)
        g_signal_handler_block (G_OBJECT (entry), changed_hook);

    /* Fill in value */
    if (value)
		gtk_entry_set_text (GTK_ENTRY (entry), value);
    else
		gtk_entry_set_text (GTK_ENTRY (entry), "");

    g_object_set_data (G_OBJECT (entry), "number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if (changed > 0)
        g_signal_handler_unblock (G_OBJECT (entry), changed);

    if (changed_hook > 0)
        g_signal_handler_unblock (G_OBJECT (entry), changed_hook);
}

/* INT */
/*
 * creates a new GtkEntry which will modify the value according to the entry for a gint
 * but made for values in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... (number, gint value content)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 gint value)
 * ex : gsb_data_account_get_element_sort (gint account_number,
 *						 gint no_column)
 *
 * \param value a gint to fill the entry
 * \param hook an optional function to execute as a handler if the
 * 	entry's contents are modified.
 * 	hook should be :
 * 		gboolean hook (GtkWidget *entry,
 * 				gpointer data)
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func (number, gint)) or NULL
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with number_for_func)
 *
 * \return a new GtkEntry
 * */
GtkWidget *gsb_autofunc_int_new (gint value,
								 GCallback hook,
								 gpointer data,
								 GCallback default_func,
								 gint number_for_func)
{
    GtkWidget * entry;
	gchar* tmpstr;

    /* first, create and fill the entry */
    entry = gtk_entry_new ();

    tmpstr = utils_str_itoa (value);
    gtk_entry_set_text (GTK_ENTRY(entry), tmpstr);
    g_free (tmpstr);

    /* set the default func :
     * the func will be send to gsb_editable_set_text by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (entry), "number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (entry),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(entry),
																	 "changed",
																	 ((GCallback) gsb_autofunc_int_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (entry),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(entry),
																	 "changed",
																	 ((GCallback) hook),
																	 data)));
    return entry;
}

/**
 * set the value in a gsb_editable_int_entry
 * a value is in 2 parts :
 * 	a string, which be showed in the entry
 * 	a number, which is used when there is a change in that entry (see gsb_autofunc_int_new)
 *
 * \param entry
 * \param value a gint to set in the entry
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_int_set_value (GtkWidget *entry,
								 gint value,
								 gint number_for_func)
{
	gchar* tmpstr;
    gulong changed;
    gulong changed_hook;

    /* Block everything */
    changed = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (entry), "changed"));
    if (changed > 0)
        g_signal_handler_block (G_OBJECT (entry), changed);

    changed_hook = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (entry), "changed-hook"));
    if (changed_hook > 0)
        g_signal_handler_block (G_OBJECT (entry), changed_hook);

    /* Fill in value */
    tmpstr = utils_str_itoa (value);
    gtk_entry_set_text (GTK_ENTRY(entry), tmpstr);
    g_free (tmpstr);

    g_object_set_data (G_OBJECT (entry), "number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if (changed > 0)
        g_signal_handler_unblock (G_OBJECT (entry), changed);

    if (changed_hook > 0)
        g_signal_handler_unblock (G_OBJECT (entry), changed_hook);
}

/**
 * erase the entry of the int_editable
 * used because if we give 0 0 to set value, will show 0 in the entry but
 * cannot erase it
 * this function will delete too the number_for_func associated with the entry
 *
 * \param entry
 *
 * \return
 * */
void gsb_autofunc_int_erase_entry (GtkWidget *entry)
{
    gulong changed;
    gulong changed_hook;

    /* Block everything */
    changed = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (entry), "changed"));
    if (changed > 0)
        g_signal_handler_block (G_OBJECT (entry), changed);

    changed_hook = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (entry), "changed-hook"));
    if (changed_hook > 0)
        g_signal_handler_block (G_OBJECT (entry), changed_hook);

    /* Fill in value */
    gtk_entry_set_text (GTK_ENTRY(entry), "");

    g_object_set_data (G_OBJECT (entry), "number_for_func", NULL);

    /* Unblock everything */
    if (changed > 0)
        g_signal_handler_unblock (G_OBJECT (entry), changed);

    if (changed_hook > 0)
        g_signal_handler_unblock (G_OBJECT (entry), changed_hook);
}

/* RADIO_BUTTON */
/**
 * Creates a new radio buttons group with two choices.  Toggling will
 * for each change, will call the corresponding given function : gsb_data_... (number, gboolean)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 gboolean yes/no)
 * ex : gsb_data_category_set_type (category_number, type)
 *
 * the 2 button's pointer are saved in the box as "button1" and "button2"
 *
 * \param choice1 First choice label
 * \param choice2 Second choice label
 * \param value A boolean that will be set to 0 or 1
 *        according to buttons toggles. (choice 2 selected means boolean = TRUE in the function)
 * \param hook An optional hook to run at each toggle
 * \param data optional data to send to hook
 * \param default_func The function to call to change the value in memory (function must be func (number, gboolean)) or NULL
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 *
 * \return a vbox with the 2 radiobuttons
 *
 */
GtkWidget *gsb_autofunc_radiobutton_new (const gchar *choice1,
										 const gchar *choice2,
										 gboolean value,
										 GCallback hook,
										 gpointer data,
										 GCallback default_func,
										 gint number_for_func)
{
    GtkWidget *button1, *button2, *vbox;

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);

    button1 = gtk_radio_button_new_with_mnemonic (NULL, choice1);
    gtk_box_pack_start (GTK_BOX(vbox), button1, FALSE, FALSE, 0);
    button2 = gtk_radio_button_new_with_mnemonic (gtk_radio_button_get_group (GTK_RADIO_BUTTON(button1)),
												  choice2);
    gtk_box_pack_start (GTK_BOX(vbox), button2, FALSE, FALSE, 0);

    if (value)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2), TRUE);
    else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button1), TRUE);

    g_object_set_data (G_OBJECT (button2), "number_for_func", GINT_TO_POINTER (number_for_func));
    g_signal_connect (G_OBJECT (button2),
					  "toggled",
					  G_CALLBACK (gsb_autofunc_checkbutton_changed),
					  default_func);

    g_object_set_data (G_OBJECT (vbox), "button1", button1);
    g_object_set_data (G_OBJECT (vbox), "button2", button2);

    if (hook)
		g_signal_connect (G_OBJECT (button2), "toggled", G_CALLBACK (hook), data);

    return vbox;
}

/* REAL */
/*
 * creates a new GtkEntry to contain a GsbReal which will modify the value according to the entry
 * but made for values in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... (number, gsb_real)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 GsbReal real)
 * ex : gsb_data_account_set_init_balance (account, real)
 *
 * \param real a GsbReal to fill the entry or NULL
 * \param hook an optional function to execute as a handler if the
 * 	entry's contents are modified.
 * 	hook should be :
 * 		gboolean hook (GtkWidget *entry,
 * 				gpointer data)
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func a function to call when something change (function must be func (number, real)) or null_real
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 * 				that number can be changed with gsb_autofunc_entry_set_value
 *
 * \return a new GtkEntry
 * */
GtkWidget *gsb_autofunc_real_new (GsbReal real,
								  GCallback hook,
								  gpointer data,
								  GCallback default_func,
								  gint number_for_func)
{
    GtkWidget *entry;
    gchar *string;

    /* first, create and fill the entry */
    entry = gtk_entry_new ();

    string = utils_real_get_string (real);
    gtk_entry_set_text (GTK_ENTRY (entry), string);
    g_free (string);

    /* set the default func :
     * the func will be send to gsb_editable_set_text by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (entry), "number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (entry),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(entry),
																	 "changed",
																	 G_CALLBACK (gsb_autofunc_real_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (entry),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(entry),
																	 "changed",
																	 G_CALLBACK (hook),
																	 data)));
    return entry;
}

/**
 * set the GsbReal in a gsb_editable_date
 * a value is in 2 parts :
 * 	a date, which be showed in the entry
 * 	a number, which is used when there is a change in that entry (see gsb_autofunc_date_new)
 *
 * \param entry
 * \param date a date to set in the entry
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_real_set (GtkWidget *entry,
							GsbReal real,
							gint number_for_func)
{
    gchar *string;
    gulong changed;
    gulong changed_hook;

    /* Block everything */
    changed = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (entry), "changed"));
    if (changed > 0)
        g_signal_handler_block (G_OBJECT (entry), changed);

    changed_hook = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (entry), "changed-hook"));
	if (changed_hook > 0)
		g_signal_handler_block (G_OBJECT (entry), changed_hook);

    /* Fill in value */
	/* fix bug 2149 si le nombre est en erreur on renvoie error_real et non null_real */
	if (real.mantissa == G_MININT64)
		string = g_strdup (ERROR_REAL_STRING);
	else
		string = utils_real_get_string (real);
    gtk_entry_set_text (GTK_ENTRY (entry), string);
    g_free (string);

    g_object_set_data (G_OBJECT (entry), "number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if (changed > 0)
        g_signal_handler_unblock (G_OBJECT (entry), changed);

    if (changed_hook > 0)
        g_signal_handler_unblock (G_OBJECT (entry), changed_hook);
}

/* SPIN */
/*
 * creates a new GtkSpinButton with a int inside (not float) which will modify the value according to the entry for a gint
 * but made for values in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... (number, gint value content)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 gint value)
 * ex : gsb_data_account_get_element_sort (gint account_number,
 *						 gint no_column)
 *
 * for now, there is standard values for the adjustement of the spin-button, change the param if necessary later
 * 	to adapt the values
 *
 * \param value a gint to fill the entry of the spin button
 * \param hook an optional function to execute as a handler if the
 * 	entry's contents are modified.
 * 	hook should be :
 * 		gboolean hook (GtkWidget *entry,
 * 				gpointer data)
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func (number, gint)) or NULL
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with number_for_func)
 *
 * \return a new GtkSpinButton
 * */
GtkWidget *gsb_autofunc_spin_new (gint value,
								  GCallback hook,
								  gpointer data,
								  GCallback default_func,
								  gint number_for_func)
{
    GtkWidget *spin_button;

    /* create and fill the spin button */
    spin_button = gtk_spin_button_new_with_range (0.0, GSB_MAX_SPIN_BUTTON, 1.0);

    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin_button), (gdouble) value);

    /* set the default func :
     * the func will be send to gsb_editable_set_text by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (spin_button), "number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (spin_button),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(spin_button),
																	 "value-changed",
																	 G_CALLBACK (gsb_autofunc_spin_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (spin_button),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(spin_button),
																	 "value-changed",
																	 G_CALLBACK (hook),
																	 data)));
    return spin_button;
}

void gsb_autofunc_spin_new_from_ui (GtkWidget *spin_button,
									gint value,
									GCallback hook,
									gpointer data,
									GCallback default_func,
									gint number_for_func)
{

    /* set range and fill the spin button */
	gtk_spin_button_set_range (GTK_SPIN_BUTTON (spin_button), 0.0, GSB_MAX_SPIN_BUTTON);
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin_button), (gdouble) value);

    /* set the default func :
     * the func will be send to gsb_editable_set_text by the data,
     * the number_for_func will be set as data for object */
    g_object_set_data (G_OBJECT (spin_button), "number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (spin_button),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(spin_button),
																	 "value-changed",
																	 G_CALLBACK (gsb_autofunc_spin_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (spin_button),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(spin_button),
																	 "value-changed",
																	 G_CALLBACK (hook),
																	 data)));
}

/**
 * set the value in a autofunc_spin entry
 * a value is in 2 parts :
 * 	a gint, which be showed in the spint button
 * 	a number, which is used when there is a change in that sping button (see gsb_autofunc_int_new)
 *
 * \param spin_button
 * \param value a gint to set in the spin_button
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_spin_set_value (GtkWidget *spin_button,
								  gint value,
								  gint number_for_func)
{
    gulong changed;
    gulong changed_hook;

    /* Block everything */
    changed = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (spin_button), "changed"));
    if (changed > 0)
        g_signal_handler_block (G_OBJECT (spin_button), changed);

    changed_hook = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (spin_button), "changed-hook"));
    if (changed_hook > 0)
        g_signal_handler_block (G_OBJECT (spin_button), changed_hook);

    /* Fill in value */
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spin_button), (gdouble) value);

    g_object_set_data (G_OBJECT (spin_button), "number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if (changed > 0)
        g_signal_handler_unblock (G_OBJECT (spin_button), changed);

    if (changed_hook > 0)
        g_signal_handler_unblock (G_OBJECT (spin_button), changed_hook);
}

/* TEXT_VIEW */
/*
 * creates a new GtkTextView which will automatickly modify the value according to the text in memory
 * in grisbi structure :
 * for each change, will call the corresponding given function : gsb_data_... (number, string content)
 * ie the target function must be :
 * 	(default_func) (gint number_for_func,
 * 			 gchar *string)
 * ex : gsb_data_bank_set_bank_note (account_number, text)
 * rem : do the same as gsb_autofunc_entry_new but for a text_view
 *
 * \param value a string to fill the text_view
 * \param hook an optional function to execute as a handler if the
 * 	text_view's contents are modified : !!! send the text_buffer, and not the text_view
 * 	hook should be :
 * 		gboolean hook (GtkTextBuffer *text_buffer,
 * 				gpointer data)
 *
 * \param data An optional pointer to pass to hooks.
 * \param default_func The function to call to change the value in memory (function must be func (number, string)) or NULL
 * \param number_for_func a gint which we be used to call default_func (will be saved as g_object_set_data with "number_for_func")
 *
 * \return a new GtkTextView
 * */
GtkWidget *gsb_autofunc_textview_new (const gchar *value,
									  GCallback hook,
									  gpointer data,
									  GCallback default_func,
									  gint number_for_func)
{
    GtkWidget *text_view;
    GtkTextBuffer *buffer;

    text_view = gsb_editable_text_view_new (value);

    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));

    /* set the default function and save the number_for_func */
    g_object_set_data (G_OBJECT (buffer), "number_for_func", GINT_TO_POINTER (number_for_func));
    if (default_func)
		g_object_set_data (G_OBJECT (buffer),
						   "changed",
						   GUINT_TO_POINTER (g_signal_connect_after (G_OBJECT(buffer),
																	 "changed",
																	 ((GCallback) gsb_autofunc_textview_changed),
																	 default_func)));
    if (hook)
		g_object_set_data (G_OBJECT (buffer),
						   "changed-hook",
						   GUINT_TO_POINTER (g_signal_connect_after  (G_OBJECT(buffer),
																	  "changed",
																	  ((GCallback) hook),
																	  data)));
    return text_view;
}

/**
 * set the value in a gsb_editable_text_area
 * a value is in 2 parts :
 * 	a string, which be showed in the text_view
 * 	a number, which is used when there is a change in that text_view (see gsb_autofunc_entry_new)
 *
 * \param text_view
 * \param value a string to set in the text_view
 * \param number_for_func the number to give to the called function when something is changed
 *
 * \return
 */
void gsb_autofunc_textview_set_value (GtkWidget *text_view,
									  const gchar *value,
									  gint number_for_func)
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
    gulong changed;
    gulong changed_hook;

    /* Block everything */
    changed = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (buffer), "changed"));
    if (changed > 0)
        g_signal_handler_block (G_OBJECT (buffer), changed);

    changed_hook = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (buffer), "changed-hook"));
    if (changed_hook > 0)
        g_signal_handler_block (G_OBJECT (buffer), changed_hook);

    /* Fill in value */
    if (value)
		gtk_text_buffer_set_text (buffer, value, -1);
    else
		gtk_text_buffer_set_text (buffer, "", -1);

    g_object_set_data (G_OBJECT (buffer), "number_for_func", GINT_TO_POINTER (number_for_func));

    /* Unblock everything */
    if (changed > 0)
        g_signal_handler_unblock (G_OBJECT (buffer), changed);

    if (changed_hook > 0)
        g_signal_handler_unblock (G_OBJECT (buffer), changed_hook);
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
