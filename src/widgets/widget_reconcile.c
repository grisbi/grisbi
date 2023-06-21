/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2022 Pierre Biava (grisbi@pierre.biava.name)                    */
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

#include <glib/gi18n.h>
#include <ctype.h>
#include <string.h>

/*START_INCLUDE*/
#include "widget_reconcile.h"
#include "dialog.h"
#include "grisbi_win.h"
#include "gsb_calendar_entry.h"
#include "gsb_data_account.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_reconcile.h"
#include "gsb_reconcile_list.h"
#include "gsb_scheduler_list.h"
#include "gsb_transactions_list.h"
#include "navigation.h"
#include "structures.h"
#include "transaction_list.h"
#include "transaction_list_sort.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetReconcilePrivate   WidgetReconcilePrivate;

struct _WidgetReconcilePrivate
{
	GtkWidget *			vbox_reconcile;

	GtkWidget *			entry_reconcile_number;
	GtkWidget *			entry_initial_date;
	GtkWidget *			entry_final_date;
	GtkWidget *			entry_initial_balance;
	GtkWidget *			entry_final_balance;
	GtkWidget *			label_initial_balance;
	GtkWidget *			label_final_balance;
	GtkWidget *			label_checking_balance;
	GtkWidget *			label_variance_balance;
	GtkWidget *			checkbutton_sort_reconcile;
	GtkWidget *			button_cancel;
	GtkWidget *			button_validate;
	GtkWidget *			label_frame_reconcile;

	/* save variable */
	gint				reconcile_save_account_display;
	gint				reconcile_save_show_marked;
	gint				reconcile_save_rows_number;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetReconcile, widget_reconcile, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * je ne vois pas à quoi sert cette fonction. conservée par précaution
 *
 * \param
 * \param
 * \param
 *
 * \return FALSE
 **/
static gboolean widget_reconcile_entry_lose_focus (GtkWidget *entry,
												   GdkEventFocus *ev,
												   gpointer data)
{
	gsb_form_check_auto_separator (entry);

	return FALSE;
}

/**
 * make sensitive or unsensitive all that could change the account
 * while we are reconciling
 *
 * \param sensitive TRUE or FALSE
 *
 * \return
 * */
static void widget_reconcile_sensitive (gboolean sensitive)
{
	gtk_widget_set_sensitive (gsb_gui_navigation_get_tree_view (), sensitive);
	grisbi_win_headings_sensitive_headings (sensitive);
}

/**
 * come here to cancel the reconciliation
 * come here too at the end of the finish reconcile to set the default values
 *
 * \param button not used
 * \param priv
 *
 * \return FALSE
 * */
static gboolean widget_reconcile_cancel (GtkWidget *button,
										 WidgetReconcilePrivate *priv)
{
	GtkWidget *reconcile_panel;
	GrisbiWinEtat *w_etat;
	GrisbiWinRun *w_run;

	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	w_run->equilibrage = FALSE;

	/* save the final balance/new date for the next time the user will try to reconcile */
	w_run->reconcile_account_number = gsb_gui_navigation_get_current_account ();
	w_run->reconcile_final_balance = g_strdup (gtk_entry_get_text (GTK_ENTRY (priv->entry_final_balance)));
	w_run->reconcile_new_date = gsb_parse_date_string (gtk_entry_get_text (GTK_ENTRY (priv->entry_final_date)));

	/* set the normal color to the date entry */
	gsb_calendar_entry_set_color (priv->entry_final_date, TRUE);

	/* restore the good sort of the list */
	if (transaction_list_sort_get_reconcile_sort ())
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_sort_reconcile), FALSE);
		gsb_reconcile_list_button_clicked (priv->checkbutton_sort_reconcile, NULL);
	}

	transaction_list_show_toggle_mark (FALSE);

	/* restore the saved data */
	w_etat->retient_affichage_par_compte = priv->reconcile_save_account_display;

	gsb_transactions_list_set_visible_rows_number (priv->reconcile_save_rows_number);

	if (priv->reconcile_save_show_marked)
	{
		gsb_data_account_set_r (gsb_gui_navigation_get_current_account (), TRUE);
		gsb_transactions_list_mise_a_jour_affichage_r (TRUE);
	}

	/* Don't display uneeded widget for now. */
	reconcile_panel = grisbi_win_get_reconcile_panel (NULL);
	gtk_widget_hide (reconcile_panel);
	widget_reconcile_sensitive (TRUE);

	return FALSE;
}

/**
 * finish the reconciliation,
 * called by a click on the finish button
 *
 * \param button
 * \param priv
 *
 * \return FALSE
 */
static gboolean widget_reconcile_validate (GtkWidget *button,
										   WidgetReconcilePrivate *priv)
{
	GtkWidget *label_last_statement;
	GSList *list_tmp_transactions;
	GDate *date;
	GDate *initial_date;
	const GDate *transactions_max_date;
	gchar *tmp_date;
	gchar *tmp_str;
	gint account_number;
	gint reconcile_number;
	gint reconcile_save_last_scheduled_convert;
	GsbReal tmp_real1;
	GsbReal tmp_real2;
	GsbReal tmp_real3;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	account_number = gsb_gui_navigation_get_current_account ();

	tmp_real1 = utils_real_get_from_string (gtk_entry_get_text (GTK_ENTRY (priv->entry_initial_balance)));
	tmp_real2 = gsb_real_add (tmp_real1, gsb_data_account_calculate_waiting_marked_balance (account_number));
	tmp_real3 = utils_real_get_from_string (gtk_entry_get_text (GTK_ENTRY (priv->entry_final_balance)));

	if (gsb_real_sub (tmp_real2, tmp_real3).mantissa != 0)
	{
		dialogue_warning_hint (_("There is a variance in balances, check that both final balance "
								 "and initial balance minus marked transactions are equal."),
							   _("Reconciliation can't be completed."));

		return FALSE;
	}

	/* get and check the reconcile name */
	reconcile_number = gsb_data_reconcile_get_number_by_name (gtk_entry_get_text (GTK_ENTRY
																				  (priv->entry_reconcile_number)));
	if (reconcile_number)
	{
		dialogue_warning_hint (_("There is already a reconciliation with that "
								 "name, you must use another name or let it free.\nIf the "
								 "reconcile name is ending by a number,\n"
								 "it will be automatically incremented."),
							   _("Reconciliation can't be completed."));

		return FALSE;
	}

	/* teste si la date de fin inclue toutes les transactions pointées */
	transactions_max_date = gsb_reconcile_get_pointed_transactions_max_date (account_number);
	if (transactions_max_date == NULL)
	{
		tmp_str = g_strdup (_("There must be at least one pointed or teletransmise operation"));
		dialogue_warning_hint (tmp_str, _("No pointed transaction"));
		g_free (tmp_str);

		/* set unsensible button_validate */
		gtk_widget_set_sensitive (GTK_WIDGET (priv->button_validate), FALSE);

		return FALSE;
	}

	/* get and save the date */
	date = gsb_calendar_entry_get_date (priv->entry_final_date);
	if (!date)
	{
		tmp_str = g_strdup_printf (_("Invalid date: '%s'"),
								   gtk_entry_get_text (GTK_ENTRY (priv->entry_final_date)));
		dialogue_warning_hint (tmp_str, ("Reconciliation can't be completed."));
		g_free (tmp_str);

		return FALSE;
	}

	if (g_date_compare (transactions_max_date, date) > 0)
	{
		if (!dialogue_yes_no (_("There are pointed transactions that occur later than "
								"the reconciliation end date.\n"
								"If you continue these transactions will be ignored.\n\n"
								"Do you want to continue?"),
							  _("Warning: the reconciliation may be incomplete!"),
							  GTK_RESPONSE_NO))
		{
			g_date_free (date);

			return FALSE;
		}
	}

	/* teste la validité de la date de fin */
	reconcile_number = gsb_data_reconcile_get_account_last_number (account_number);
	if (reconcile_number)
	{
		initial_date = g_date_copy (gsb_data_reconcile_get_final_date (reconcile_number));
		if (g_date_compare (initial_date, date) >= 0)
		{
			tmp_date = gsb_format_gdate (date);
			tmp_str = g_strdup_printf (_("Invalid date: '%s'"), tmp_date);
			g_free(tmp_date);
			dialogue_warning_hint (tmp_str, _("Reconciliation can't be completed."));
			g_free (tmp_str);
			g_date_free (initial_date);

			return FALSE;
		}
	}
	else
	{
		initial_date = gsb_calendar_entry_get_date (priv->entry_initial_date);
		if (!initial_date)
		{
			tmp_str = g_strdup (_("You can set the initial date of the reconciliation in the preferences."));
			dialogue_warning (tmp_str);
			g_free (tmp_str);
		}
	}

	if (!strlen (gtk_entry_get_text (GTK_ENTRY (priv->entry_reconcile_number))))
	{
		dialogue_warning_hint (_("You need to set a name to the reconciliation ; at least, set a number,\n"
								 "it will be automatically incremented later"),
							   _("Reconciliation can't be completed."));

		return FALSE;
	}

	/* restore the good sort of the list */
	if (transaction_list_sort_get_reconcile_sort ())
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_sort_reconcile), FALSE);
		gsb_reconcile_list_button_clicked (priv->checkbutton_sort_reconcile, NULL);
	}

	label_last_statement = grisbi_win_get_label_last_statement ();
	tmp_date = gsb_format_gdate (date);
	tmp_str = g_strdup_printf (_("Last statement: %s"), tmp_date);
	g_free(tmp_date);
	gtk_label_set_text (GTK_LABEL (label_last_statement), tmp_str);
	g_free (tmp_str);

	/* create the new reconcile structure */
	reconcile_number = gsb_data_reconcile_new (gtk_entry_get_text (GTK_ENTRY (priv->entry_reconcile_number)));
	gsb_data_reconcile_set_account (reconcile_number, account_number);

	/* set the variables of the reconcile */
	gsb_data_reconcile_set_final_date (reconcile_number, date);
	g_date_free (date);

	gsb_data_reconcile_set_init_date (reconcile_number, initial_date);
	g_free (initial_date);

	gsb_data_reconcile_set_init_balance (reconcile_number, tmp_real1);

	gsb_data_reconcile_set_final_balance (reconcile_number, tmp_real3);

	/* modify the reconciled transactions */
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while (list_tmp_transactions)
	{
		gint transaction_number_tmp;

		transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions->data);

		if (gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number
			 && (gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_POINTEE
				 || gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_TELEPOINTEE))
		{
			gsb_data_transaction_set_marked_transaction (transaction_number_tmp, OPERATION_RAPPROCHEE);
			gsb_data_transaction_set_reconcile_number (transaction_number_tmp, reconcile_number);
		}

		list_tmp_transactions = list_tmp_transactions->next;
	}

	/* update the P and T to R in the list */
	transaction_list_update_element (ELEMENT_MARK);

	/* update the reconcile number if necessary */
	transaction_list_update_element (ELEMENT_RECONCILE_NB);

	w_run->mise_a_jour_liste_comptes_accueil = TRUE;

	/* go back to the normal transactions list */
	widget_reconcile_cancel (NULL, priv);

	/* reset records in run: to do after widget_reconcile_cancel */
	g_free (w_run->reconcile_final_balance);
	if (w_run->reconcile_new_date)
		g_date_free (w_run->reconcile_new_date);
	w_run->reconcile_final_balance = NULL;
	w_run->reconcile_new_date = NULL;
	w_run->reconcile_account_number = -1;

	gsb_file_set_modified (TRUE);

	reconcile_save_last_scheduled_convert = gsb_reconcile_get_last_scheduled_transaction ();
	if (reconcile_save_last_scheduled_convert)
	{
		gsb_gui_navigation_set_selection (GSB_SCHEDULER_PAGE, 0, 0);
		gsb_scheduler_list_select (reconcile_save_last_scheduled_convert);
		gsb_scheduler_list_edit_transaction (reconcile_save_last_scheduled_convert);
	}

	return FALSE;
}

/**
 * update the labels according to the value in the account structure and the entries
 * so all the calculs must have been done before (for marked transactions)
 *
 * \param
 *
 * \return FALSE
 * */
static void widget_reconcile_update_amounts_labels (GtkWidget *entry,
													WidgetReconcilePrivate *priv)
{
	const gchar *initial_balance;
	const gchar *final_balance;
	gchar *tmp_string;
	gchar* tmp_str;
	gint account_number;
	gint currency_number;
	gboolean valide;
	GsbReal amount;

	/* first get the current account number */
	account_number = gsb_gui_navigation_get_current_account ();

	/* fill the labels corresponding to the balances */
	initial_balance = gtk_entry_get_text (GTK_ENTRY (priv->entry_initial_balance));
	gtk_label_set_text (GTK_LABEL (priv->label_initial_balance), initial_balance);

	if (entry)
	{
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

			return;
		}
	}

	currency_number = gsb_data_account_get_currency (account_number);
	amount = utils_real_get_calculate_entry (priv->entry_final_balance);
	final_balance = utils_real_get_string_with_currency (amount, currency_number, FALSE);
	gtk_label_set_text (GTK_LABEL (priv->label_final_balance), final_balance);

	/* set the marked balance amount,
	 * this is what we mark as P while reconciling, so it's the total marked balance
	 * - the initial marked balance */
	tmp_string = utils_real_get_string_with_currency (gsb_data_account_calculate_waiting_marked_balance
													  (account_number),
													  currency_number,
													  FALSE);
	gtk_label_set_text (GTK_LABEL (priv->label_checking_balance), tmp_string);
	g_free (tmp_string);

	/* calculate the variation balance and show it */
	amount = gsb_real_sub (gsb_real_add (utils_real_get_from_string (initial_balance),
										 gsb_data_account_calculate_waiting_marked_balance (account_number)),
						   utils_real_get_from_string (final_balance));

	tmp_str = utils_real_get_string_with_currency (amount, currency_number, FALSE);
	gtk_label_set_text (GTK_LABEL (priv->label_variance_balance), tmp_str);
	g_free (tmp_str);

	if (amount.mantissa)
		gtk_widget_set_sensitive (GTK_WIDGET (priv->button_validate), FALSE);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (priv->button_validate), TRUE);
}

/**
 * Création du widget reconcile
 *
 * \param
 * \return
 **/
static void widget_reconcile_setup_widget (WidgetReconcile *widget)
{
	WidgetReconcilePrivate *priv;

	devel_debug (NULL);
	priv = widget_reconcile_get_instance_private (widget);

	gsb_calendar_entry_new_from_ui (priv->entry_initial_date, FALSE);
	gsb_calendar_entry_new_from_ui (priv->entry_final_date, FALSE);

	/* set the old balance, an entry the first time, will be unsensitive after */
	gtk_widget_set_hexpand (priv->entry_initial_balance, TRUE);
	g_signal_connect (G_OBJECT (priv->entry_initial_balance),
					  "changed",
					  G_CALLBACK (widget_reconcile_update_amounts),
					  priv);
	g_signal_connect (G_OBJECT (priv->entry_initial_balance),
					  "focus-out-event",
					  G_CALLBACK (widget_reconcile_entry_lose_focus),
					  NULL);

	/* set the new balance */
	gtk_widget_set_hexpand (priv->entry_final_balance, TRUE);
	g_signal_connect (G_OBJECT (priv->entry_final_balance),
					  "changed",
					  G_CALLBACK (widget_reconcile_update_amounts),
					  priv);
	g_signal_connect (G_OBJECT (priv->entry_final_balance),
					  "focus-out-event",
					  G_CALLBACK (widget_reconcile_entry_lose_focus),
					  NULL);

	/* set signal for checkbutton checkbutton_sort_reconcile */
	g_signal_connect (G_OBJECT (priv->checkbutton_sort_reconcile),
					  "clicked",
					  G_CALLBACK (gsb_reconcile_list_button_clicked),
					  NULL);

	/* set signals for buttons */
	g_signal_connect (G_OBJECT (priv->button_cancel),
					  "clicked",
					  G_CALLBACK (widget_reconcile_cancel),
					  priv);

	g_signal_connect (G_OBJECT (priv->button_validate),
					  "clicked",
					  G_CALLBACK (widget_reconcile_validate),
					  priv);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_reconcile_init (WidgetReconcile *widget)
{
	gtk_widget_init_template (GTK_WIDGET (widget));
}

static void widget_reconcile_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_reconcile_parent_class)->dispose (object);
}

static void widget_reconcile_class_init (WidgetReconcileClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_reconcile_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_reconcile.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, vbox_reconcile);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, label_frame_reconcile);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, button_cancel);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, button_validate);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, checkbutton_sort_reconcile);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, entry_reconcile_number);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, entry_initial_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, entry_final_date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, entry_initial_balance);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, entry_final_balance);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, label_checking_balance);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, label_initial_balance);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, label_final_balance);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetReconcile, label_variance_balance);
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
WidgetReconcile *widget_reconcile_new (void)
{
	WidgetReconcile *widget;

	widget = g_object_new (WIDGET_RECONCILE_TYPE, NULL);
	widget_reconcile_setup_widget (widget);

	return widget;
}

/**
 * Build the new label for the reconciliation, given the old one.
 * The expected format is NAME+NUMBER, so this function returns
 * a newly allocated string whose format is NAME+(NUMBER+1). It
 * preserves leading '0' for the NUMBER string.
 *
 * If this is the first label building (first reconciliation for
 * this account), then the function returns a standard string
 * of the account name (lower case) + '-1'.
 *
 * \param reconcile_number
 *
 * \return the new string label
 */
gchar *widget_reconcile_build_label (gint reconcile_number,
									 gint account_number)
{
	gchar *tmp;
	gchar *old_label;
	gchar *new_label;
	size_t __size;
	size_t __size_expand;
	int __expand;
	int __reconcile_number;

	/* old_label = NAME + NUMBER */
	old_label = g_strdup (gsb_data_reconcile_get_name (reconcile_number));

	/* return account NAME + '1' */
	if (!old_label)
	{
		tmp = gsb_data_account_get_name (account_number);
		new_label = g_strconcat (tmp, "-1", NULL);
		tmp = new_label;
		while (*tmp != '\0')
		{
			if (*tmp == ' ')
				*tmp = '-';
			else
				*tmp = tolower (*tmp);
			tmp ++;
		}
		return new_label;
	}

	/* we try to find some digits at the end of the name,
	 * if found, get the biggest number until we find a non digit character */
	__expand = 1;
	tmp = old_label + (strlen (old_label) - 1) * sizeof (gchar);

	while (tmp >= old_label)
	{
		if (!isdigit (tmp[0]))
			break;

		if (*tmp != '9')
			__expand = 0;
		tmp--;
	}

	tmp ++; /* step forward to the first digit */

	__reconcile_number = utils_str_atoi (tmp) + 1;

	/* if stage 99->100 for example,
	 * then we have to allocate one more byte */
	__size_expand = strlen (tmp) + __expand;
	/* format string for the output (according NUMBER string length) */

	/* close the NAME string */
	*tmp = 0;
	/* NAME + NUMBER + '\0' */
	__size = __size_expand + strlen (old_label) * sizeof (gchar) + 1;
	new_label = g_malloc0 (__size * sizeof (gchar));
	sprintf (new_label, "%s%*d", old_label, (int)__size_expand, __reconcile_number);

	/* replace ' ' by '0' in number */
	tmp = new_label + strlen (old_label) * sizeof (gchar);
	while (*tmp == ' ')
		*tmp++ = '0';

	g_free (old_label);

	return new_label;
}

/**
 * start the reconciliation, called by a click on the
 * reconcile button
 *
 * \param button the button we click to come here
 * \param null not used
 *
 * \return FALSE
 * */
gboolean widget_reconcile_run_reconciliation (GtkWidget *button,
											  gpointer null)
{
	GtkWidget *reconcile_panel;
	GDate *date;
	gchar *label;
	gchar *string;
	gchar* tmp_str;
	gint account_number;
	gint reconcile_number;
	GrisbiWinEtat *w_etat;
	GrisbiWinRun *w_run;
	WidgetReconcilePrivate *priv;

	devel_debug (NULL);
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	account_number = gsb_gui_navigation_get_current_account ();
	reconcile_number = gsb_data_reconcile_get_account_last_number (account_number);

	reconcile_panel = grisbi_win_get_reconcile_panel (NULL);
	priv = widget_reconcile_get_instance_private (WIDGET_RECONCILE (reconcile_panel));

	/* set nom du rapprochement */
	label = widget_reconcile_build_label (reconcile_number, account_number);
	gtk_entry_set_text (GTK_ENTRY (priv->entry_reconcile_number), label);
	g_free (label);

	/* reset records in run structure if user has changed of account */
	if (w_run->reconcile_account_number != account_number)
	{
		g_free (w_run->reconcile_final_balance);
		if (w_run->reconcile_new_date)
			g_date_free (w_run->reconcile_new_date);
		w_run->reconcile_final_balance = NULL;
		w_run->reconcile_new_date = NULL;
		w_run->reconcile_account_number = -1;
	}

	/* set last input date/amount if available */
	if (w_run->reconcile_new_date)
	{
		date = w_run->reconcile_new_date;
	}
	else
	{
		/* increase the last date of 1 month */
		date = gsb_date_copy (gsb_data_reconcile_get_final_date (reconcile_number));
		if (date)
		{
			GDate *today;

			/* it's not the first reconciliation, init and make unsensitive the the initial date entry */
			string = gsb_format_gdate (date);
			gtk_entry_set_text (GTK_ENTRY (priv->entry_initial_date), string);
			gtk_widget_set_sensitive (GTK_WIDGET (priv->entry_initial_date), FALSE);

			g_free (string);
			g_date_add_months (date, 1);

			/* if w_etat->reconcile_end_date or the new date is after today, set today */
			today = gdate_today();
			if (w_etat->reconcile_end_date || g_date_compare (date, today) > 0)
			{
				g_date_free (date);
				date = gdate_today();
			}
			else
				g_date_free (today);

			/* set the old balance and unsensitive the old balance entry */
			tmp_str = utils_real_get_string (gsb_data_reconcile_get_final_balance (reconcile_number));
			gtk_entry_set_text (GTK_ENTRY (priv->entry_initial_balance), tmp_str);
			g_free (tmp_str);
			gtk_widget_set_sensitive (GTK_WIDGET (priv->entry_initial_balance), FALSE);
		}
		else
		{
			/* it's the first reconciliation, init and make sensitive the the initial date entry */
			gtk_entry_set_placeholder_text (GTK_ENTRY (priv->entry_initial_date), _("None"));
			gtk_widget_set_sensitive (priv->entry_initial_date, TRUE);

			date = gdate_today();

			/* set the initial balance and make sensitive the old balance to change it if necessary */
			tmp_str = utils_real_get_string (gsb_data_account_get_init_balance (account_number, -1));
			gtk_entry_set_text (GTK_ENTRY (priv->entry_initial_balance), tmp_str);
			g_free (tmp_str);
			gtk_widget_set_sensitive (GTK_WIDGET (priv->entry_initial_balance), TRUE);
		}
	}

	string = gsb_format_gdate (date);
	gtk_entry_set_text (GTK_ENTRY (priv->entry_final_date), string);
	g_free (string);
	g_date_free (date);

	/* set last input amount if available and if the account is the good one */
	if (w_run->reconcile_final_balance)
		gtk_entry_set_text (GTK_ENTRY (priv->entry_final_balance), w_run->reconcile_final_balance);
	else
		gtk_entry_set_text (GTK_ENTRY (priv->entry_final_balance), "");
	g_free (w_run->reconcile_final_balance);

	/* set the frame title */
	tmp_str = g_markup_printf_escaped (_(" <b>%s reconciliation</b> "), gsb_data_account_get_name (account_number));
	gtk_label_set_markup (GTK_LABEL (priv->label_frame_reconcile), tmp_str);
	g_free (tmp_str);

	/* we go to the reconciliation mode */
	w_run->equilibrage = TRUE;

	/* set all the balances for reconciliation */
	widget_reconcile_update_amounts_labels (NULL, priv);

	/* set the transactions list to reconciliation mode  only change the current account */
	priv->reconcile_save_account_display = w_etat->retient_affichage_par_compte;
	w_etat->retient_affichage_par_compte = 1;

	/* hide the marked R transactions */
	priv->reconcile_save_show_marked = gsb_data_account_get_r (account_number);
	if (priv->reconcile_save_show_marked)
	{
		gsb_data_account_set_r (account_number, FALSE);
		gsb_transactions_list_mise_a_jour_affichage_r (FALSE);
	}

	/* 1 line on the transaction list */
	priv->reconcile_save_rows_number = gsb_data_account_get_nb_rows (account_number);
	if (priv->reconcile_save_rows_number != 1)
		gsb_transactions_list_set_visible_rows_number (1);

	/* sort by method of payment if in conf */
	if (gsb_data_account_get_reconcile_sort_type (account_number))
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_sort_reconcile), TRUE);

	gtk_widget_show_all (GTK_WIDGET (reconcile_panel));

	transaction_list_show_toggle_mark (TRUE);

	/* unsensitive all that could change the account number */
	widget_reconcile_sensitive (FALSE);

	gtk_widget_grab_focus (GTK_WIDGET (priv->entry_reconcile_number));

	return FALSE;
}

/**
 * update the labels according to the value in the account structure and the entries
 * so all the calculs must have been done before (for marked transactions)
 *
 * \param
 *
 * \return FALSE
 * */
void widget_reconcile_update_amounts (void)
{
	GtkWidget *reconcile_panel;
	WidgetReconcilePrivate *priv;

	reconcile_panel = grisbi_win_get_reconcile_panel (NULL);
	priv = widget_reconcile_get_instance_private (WIDGET_RECONCILE (reconcile_panel));

	widget_reconcile_update_amounts_labels (NULL, priv);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void widget_reconcile_set_active_sort_checkbutton (gboolean active)
{
	GtkWidget *reconcile_panel;
	WidgetReconcilePrivate *priv;

	reconcile_panel = grisbi_win_get_reconcile_panel (NULL);
	priv = widget_reconcile_get_instance_private (WIDGET_RECONCILE (reconcile_panel));

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_sort_reconcile), active);
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

