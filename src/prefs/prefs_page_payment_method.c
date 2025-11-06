/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org                                                */
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

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "prefs_page_payment_method.h"
#include "dialog.h"
#include "gsb_autofunc.h"
#include "gsb_combo_box.h"
#include "gsb_data_account.h"
#include "gsb_data_form.h"
#include "gsb_data_payment.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_form.h"
#include "gsb_form_widget.h"
#include "gsb_payment_method.h"
#include "prefs_page_reconcile_sort.h"
#include "structures.h"
#include "transaction_list.h"
#include "utils.h"
#include "utils_prefs.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _PrefsPagePaymentMethodPrivate   PrefsPagePaymentMethodPrivate;

struct _PrefsPagePaymentMethodPrivate
{
	GtkWidget *			vbox_payment_method;

	GtkWidget *			button_payment_add;
	GtkWidget *			button_payment_collapse;
	GtkWidget *			button_collapse_all;
	GtkWidget *			button_expand_all;
	GtkWidget *			button_payment_remove;
	GtkWidget *			checkbutton_payment_auto;
	GtkWidget *			checkbutton_payment_entry;
	GtkWidget *			combo_payment_type;
	GtkWidget *			entry_payment_name;
	GtkWidget *			spinbutton_payment_number;
	GtkWidget *			treeview_payment;

	GtkWidget *			dialog_payment_switch;
	GtkWidget *			combo_dialog_payment;
	GtkWidget *			label_dialog_with_combo;
	GtkWidget *			label_dialog_without_combo;
	GtkWidget *			box_dialog_with_combo;
};

G_DEFINE_TYPE_WITH_PRIVATE (PrefsPagePaymentMethod, prefs_page_payment_method, GTK_TYPE_BOX)

/** Columns for payment methods tree */
enum payment_methods_columns {
	PAYMENT_METHOD_NAME_COLUMN = 0,
	PAYMENT_METHOD_NUMBERING_COLUMN,
	PAYMENT_METHOD_DEFAULT_COLUMN,
	PAYMENT_METHOD_TYPE_COLUMN,
	PAYMENT_METHOD_VISIBLE_COLUMN,
	PAYMENT_METHOD_ACTIVABLE_COLUMN,
	PAYMENT_METHOD_NUMBER_COLUMN,
	PAYMENT_METHOD_ACCOUNT_COLUMN,
	NUM_PAYMENT_METHOD_COLUMNS
};

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * called to switch the transactions from a method of payment to another
 * popup a dialog to ask for what method of payment we want to move
 *
 * \param payment_number the method of payment we come from
 *
 * \return TRUE done, FALSE cancel
 **/
static gboolean prefs_page_payment_method_switch_payment (PrefsPagePaymentMethod *page,
                                                          gint payment_number)
{
    GSList *tmp_list;
    gint account_number;
    gint new_payment_number;
    gint resultat;
	PrefsPagePaymentMethodPrivate *priv;

	priv = prefs_page_payment_method_get_instance_private (page);
	gtk_widget_show (priv->dialog_payment_switch);

    /* create first the combo_box with the possible target to the method of payment,
     * if no target, say it in the label and let that function choose if it has to be shown or not */
    account_number = gsb_data_payment_get_account_number (payment_number);
    if (!gsb_payment_method_create_combo_list (priv->combo_dialog_payment,
						gsb_data_payment_get_sign (payment_number),
						account_number,
						payment_number, FALSE))
	{
		dialogue_error (_("Problem while filling the target method of payment to delete it.\n"
		                   "This is not normal, please contact the Grisbi team."));

		return FALSE;
	}

    if (gtk_widget_get_visible (priv->combo_dialog_payment))
	{
		gtk_widget_show (priv->box_dialog_with_combo);
		gtk_widget_show (priv->label_dialog_with_combo);
		gtk_widget_hide (priv->label_dialog_without_combo);
	}
	else
	{
		gtk_widget_hide (priv->box_dialog_with_combo);
		gtk_widget_hide (priv->label_dialog_with_combo);
		gtk_widget_show (priv->label_dialog_without_combo);
	}

    resultat = gtk_dialog_run (GTK_DIALOG (priv->dialog_payment_switch));

    if (resultat != GTK_RESPONSE_OK)
    {
		if (GTK_IS_DIALOG (priv->dialog_payment_switch))
			gtk_widget_hide (priv->dialog_payment_switch);

		return FALSE;
    }

    /* Find new method of payment */
    new_payment_number = gsb_payment_method_get_selected_number (priv->combo_dialog_payment);

    /* Then, we change the method of payment for related operations. */
    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
    while (tmp_list)
    {
		TransactionStruct *transaction;

		transaction = tmp_list->data;
		if (transaction->method_of_payment_number == payment_number)
			gsb_data_transaction_set_method_of_payment_number (transaction->transaction_number, new_payment_number);

		tmp_list = tmp_list->next;
    }

    /* change too for the scheduled transactions */
    tmp_list = gsb_data_scheduled_get_scheduled_list ();
    while (tmp_list)
    {
		ScheduledStruct *scheduled;

		scheduled = tmp_list->data;
		if (scheduled->method_of_payment_number == payment_number)
			gsb_data_scheduled_set_method_of_payment_number (scheduled->scheduled_number, new_payment_number);

		tmp_list = tmp_list->next;
    }

    /* change for the transactions tree_view */
    transaction_list_update_element (ELEMENT_PAYMENT_TYPE);

    gtk_widget_hide (priv->dialog_payment_switch);

	return TRUE;
}

/**
 * called to switch the transactions from a method of payment to another
 * popup a dialog to ask for what method of payment we want to move
 *
 * \param payment_number the method of payment we come from
 *
 * \return TRUE done, FALSE cancel
 **/
static void prefs_page_payment_method_init_switch_dialog (PrefsPagePaymentMethod *page,
                                                          GrisbiPrefs *win)
{
	PrefsPagePaymentMethodPrivate *priv;

	priv = prefs_page_payment_method_get_instance_private (page);
	gtk_window_set_transient_for (GTK_WINDOW (priv->dialog_payment_switch), GTK_WINDOW (win));
	gtk_window_set_destroy_with_parent (GTK_WINDOW (priv->dialog_payment_switch), TRUE);
	gtk_window_set_modal (GTK_WINDOW (priv->dialog_payment_switch), TRUE);
    gtk_widget_set_size_request (priv->dialog_payment_switch, 600, 400);

}

/**
 * sensitive unsensitive details of payment method and remove button
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_payment_method_sensitive_widgets (PrefsPagePaymentMethodPrivate *priv,
                                                         gboolean sensitive)
{
	gtk_widget_set_sensitive (priv->button_payment_remove, sensitive);
	gtk_widget_set_sensitive (priv->checkbutton_payment_auto, sensitive);
    gtk_widget_set_sensitive (priv->checkbutton_payment_entry, sensitive);
	gtk_widget_set_sensitive (priv->combo_payment_type, sensitive);
	gtk_widget_set_sensitive (priv->entry_payment_name, sensitive);
	gtk_widget_set_sensitive (priv->spinbutton_payment_number, sensitive);
}

/**
 * set bouton in mode collapse when select a payment method
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_payment_method_set_collapse_mode (GtkWidget *button,
                                                         PrefsPagePaymentMethodPrivate *priv)
{
	if (gtk_widget_get_no_show_all (priv->button_expand_all))
	{
		gtk_widget_set_no_show_all (priv->button_expand_all, FALSE);
	}
	else
	{
		gtk_widget_set_no_show_all (priv->button_collapse_all, FALSE);
	}

	gtk_widget_hide (priv->button_expand_all);
	gtk_widget_show_all (priv->button_collapse_all);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void prefs_page_payment_method_button_collapse_expand_clicked (GtkToggleButton *button,
                                                                      PrefsPagePaymentMethod *page)
{
	PrefsPagePaymentMethodPrivate *priv;

	priv = prefs_page_payment_method_get_instance_private (page);
	if (gtk_widget_get_no_show_all (priv->button_expand_all))
	{
		gtk_widget_set_no_show_all (priv->button_expand_all, FALSE);
	}
	else
	{
		gtk_widget_set_no_show_all (priv->button_collapse_all, FALSE);
	}

	if (gtk_toggle_button_get_active (button))
	{
		gtk_widget_hide (priv->button_expand_all);
		gtk_widget_show_all (priv->button_collapse_all);
		gtk_tree_view_expand_all (GTK_TREE_VIEW (priv->treeview_payment));
	}
	else
	{
		gtk_widget_show_all (priv->button_expand_all);
		gtk_widget_hide (priv->button_collapse_all);
		gtk_tree_view_collapse_all (GTK_TREE_VIEW (priv->treeview_payment));
	}
}

/**
 * Callback used when a payment method is selected in payment methods
 * list.
 *
 * \param selection the tree selection
 * \param page
 *
 * \return
 **/
static void prefs_page_payment_method_row_selected (GtkTreeSelection *selection,
                                                    PrefsPagePaymentMethodPrivate *priv)
{
	GtkTreeModel *model;
    GtkTreeIter iter;
    gboolean good;

    good = gtk_tree_selection_get_selected (selection, &model, &iter);
    if (good)
    {
		GtkTreePath *path;
		gint payment_number;

		path = gtk_tree_model_get_path (model, &iter);
		gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->treeview_payment), path, TRUE);
		gtk_tree_path_free (path);
		gtk_tree_model_get (model, &iter, PAYMENT_METHOD_NUMBER_COLUMN, &payment_number, -1);

		if (payment_number)
		{
			gint show_entry;

			show_entry = gsb_data_payment_get_show_entry (payment_number);
			prefs_page_payment_method_sensitive_widgets (priv, TRUE);

			/* Filling entries */
			gsb_autofunc_entry_set_value (priv->entry_payment_name,
			                              gsb_data_payment_get_name (payment_number),
			                              payment_number);
			gsb_autofunc_spin_set_value (priv->spinbutton_payment_number,
			                             gsb_data_payment_get_last_number_to_int (payment_number),
			                             payment_number);
			gsb_autofunc_checkbutton_set_value (priv->checkbutton_payment_entry, show_entry, payment_number);
			gsb_autofunc_checkbutton_set_value (priv->checkbutton_payment_auto,
			                                    gsb_data_payment_get_automatic_numbering (payment_number),
			                                    payment_number);
			gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_payment_type),
			                         gsb_data_payment_get_sign (payment_number));

			gtk_widget_set_sensitive (priv->checkbutton_payment_auto, show_entry);

		}
		else
		{
			/* Blanking entries */
			gsb_autofunc_entry_set_value (priv->entry_payment_name, NULL, 0);
			gsb_autofunc_spin_set_value (priv->spinbutton_payment_number, 0, 0);
			gsb_autofunc_checkbutton_set_value (priv->checkbutton_payment_entry, FALSE, 0);
			gsb_autofunc_checkbutton_set_value (priv->checkbutton_payment_auto, FALSE, 0);
			gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_payment_type), 0);
			prefs_page_payment_method_set_collapse_mode (priv->button_payment_collapse, priv);

			prefs_page_payment_method_sensitive_widgets (priv, FALSE);
		}
	}
	else
	{
		/* Blanking entries */
		gsb_autofunc_entry_set_value (priv->entry_payment_name, NULL, 0);
		gsb_autofunc_spin_set_value (priv->spinbutton_payment_number, 0, 0);
		gsb_autofunc_checkbutton_set_value (priv->checkbutton_payment_entry, FALSE, 0);
		gsb_autofunc_checkbutton_set_value (priv->checkbutton_payment_auto, FALSE, 0);
		gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_payment_type), 0);

		prefs_page_payment_method_sensitive_widgets (priv, FALSE);
    }
}

/**
 * Callback called when toggle the default payment method
 *
 * \param cell
 * \param path_str
 * \param tree_view
 *
 * \return FALSE
 **/
static void prefs_page_payment_method_cell_default_toggled (GtkCellRendererToggle *cell,
															gchar *path_str,
															GtkTreeView *tree_view)
{
	GtkTreePath *path;
	GtkTreeIter iter, parent, child;
	gboolean toggle_item;
	gint payment_number;
	GtkTreeModel *model;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	path = gtk_tree_path_new_from_string (path_str);

	/* get toggled iter */
	gtk_tree_model_get_iter (GTK_TREE_MODEL(model), &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL(model),
						&iter,
						PAYMENT_METHOD_DEFAULT_COLUMN, &toggle_item,
						PAYMENT_METHOD_NUMBER_COLUMN, &payment_number,
						-1);

	if (gsb_data_payment_get_sign (payment_number) == GSB_PAYMENT_DEBIT)
		gsb_data_account_set_default_debit (gsb_data_payment_get_account_number (payment_number),
							 payment_number);
	else
	{
		if (gsb_data_payment_get_sign (payment_number) == GSB_PAYMENT_CREDIT)
			gsb_data_account_set_default_credit (gsb_data_payment_get_account_number (payment_number),
							 payment_number);
	}

	if (!toggle_item)
	{
		gtk_tree_model_iter_parent (GTK_TREE_MODEL(model), &parent, &iter);

		if (gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &parent))
		{
			do
			{
				gtk_tree_store_set (GTK_TREE_STORE (model),
									&child,
									PAYMENT_METHOD_DEFAULT_COLUMN, FALSE,
									-1);
			}
			while (gtk_tree_model_iter_next (GTK_TREE_MODEL(model), &child));
		}
		else
		{
			/* Should not happen theorically */
			dialogue_error_brain_damage () ;
		}

		/* set new value */
		gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
							PAYMENT_METHOD_DEFAULT_COLUMN, 1,
							-1);
	}

	/* clean up */
	gtk_tree_path_free (path);
	utils_prefs_gsb_file_set_modified ();
}

/**
 * Fill the `model' GtkTreeModel with all payment methods known.  They
 * are organized by account and then my method_ptr of payment method: Debit,
 * Credit, Neutral.
 *
 *  \param tree_view
 *
 *  \return
 **/
static void prefs_page_payment_method_fill (GtkWidget *tree_view)
{
	GtkTreeModel *model;
	GSList *list_tmp;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
	if (!model)
		return;

	gtk_tree_store_clear (GTK_TREE_STORE (model));

	/* Fill tree, iter over with accounts */
	list_tmp = gsb_data_account_get_list_accounts ();

	while (list_tmp)
	{
		gint account_number;
		GSList *payment_list;
		GtkTreeIter account_iter;
		GtkTreeIter credit_iter;
		GtkTreeIter debit_iter;

		account_number = gsb_data_account_get_no_account (list_tmp->data);

		gtk_tree_store_append (GTK_TREE_STORE (model), &account_iter, NULL);
		gtk_tree_store_set (GTK_TREE_STORE (model),
							&account_iter,
							PAYMENT_METHOD_NAME_COLUMN, gsb_data_account_get_name (account_number),
							PAYMENT_METHOD_NUMBERING_COLUMN, NULL,
							PAYMENT_METHOD_TYPE_COLUMN, 0,
							PAYMENT_METHOD_DEFAULT_COLUMN, FALSE,
							PAYMENT_METHOD_ACTIVABLE_COLUMN, FALSE,
							PAYMENT_METHOD_VISIBLE_COLUMN, FALSE,
							PAYMENT_METHOD_NUMBER_COLUMN, NULL,
							PAYMENT_METHOD_ACCOUNT_COLUMN, account_number,
							-1);

		/* Create the "Debit" node */
		gtk_tree_store_append (GTK_TREE_STORE (model), &debit_iter, &account_iter);
		gtk_tree_store_set (GTK_TREE_STORE (model),
							&debit_iter,
							PAYMENT_METHOD_NAME_COLUMN, _("Debit"),
							PAYMENT_METHOD_NUMBERING_COLUMN, NULL,
							PAYMENT_METHOD_TYPE_COLUMN, 0,
							PAYMENT_METHOD_DEFAULT_COLUMN, FALSE,
							PAYMENT_METHOD_ACTIVABLE_COLUMN, FALSE,
							PAYMENT_METHOD_VISIBLE_COLUMN, FALSE,
							PAYMENT_METHOD_NUMBER_COLUMN, NULL,
							PAYMENT_METHOD_ACCOUNT_COLUMN, account_number,
							-1);

		/* Create the "Credit" node */
		gtk_tree_store_append (GTK_TREE_STORE (model), &credit_iter, &account_iter);
		gtk_tree_store_set (GTK_TREE_STORE (model),
							&credit_iter,
							PAYMENT_METHOD_NAME_COLUMN, _("Credit"),
							PAYMENT_METHOD_NUMBERING_COLUMN, NULL,
							/* This is a hack: account number is put in Debit/Credit nodes */
							PAYMENT_METHOD_TYPE_COLUMN, 0,
							PAYMENT_METHOD_DEFAULT_COLUMN, FALSE,
							PAYMENT_METHOD_ACTIVABLE_COLUMN, FALSE,
							PAYMENT_METHOD_VISIBLE_COLUMN, FALSE,
							PAYMENT_METHOD_NUMBER_COLUMN, NULL,
							PAYMENT_METHOD_ACCOUNT_COLUMN, account_number,
							-1);


		/* Iter over account payment methods */
		payment_list = gsb_data_payment_get_payments_list ();
		while (payment_list)
		{
			gint payment_number;
			GtkTreeIter *parent_iter = NULL;
			GtkTreeIter method_iter;
			gboolean isdefault;
			gchar *number;

			payment_number = gsb_data_payment_get_number (payment_list->data);

			/* check if we have to show this payment here */
			if (gsb_data_payment_get_account_number (payment_number) != account_number)
			{
				payment_list = payment_list->next;
				continue;
			}

			/* set if default value */
			if (payment_number == gsb_data_account_get_default_debit (account_number)
			 ||
			 payment_number == gsb_data_account_get_default_credit (account_number))
				isdefault = 1;
			else
				isdefault = 0;

			/* look for the parent iter according to the sign */
			switch (gsb_data_payment_get_sign (payment_number))
			{
				case GSB_PAYMENT_NEUTRAL:
					parent_iter = &account_iter;
					break;
				case GSB_PAYMENT_DEBIT:
					parent_iter = &debit_iter;
					break;
				case GSB_PAYMENT_CREDIT:
					parent_iter = &credit_iter;
					break;
			}

			/* set the last number */
			if (gsb_data_payment_get_automatic_numbering (payment_number))
				number = g_strdup (gsb_data_payment_get_last_number (payment_number));
			else
				number = g_strdup ("");

			/* Insert a child node */
			gtk_tree_store_append (GTK_TREE_STORE (model), &method_iter, parent_iter);
			gtk_tree_store_set (GTK_TREE_STORE (model),
								&method_iter,
								PAYMENT_METHOD_NAME_COLUMN, gsb_data_payment_get_name (payment_number),
								PAYMENT_METHOD_NUMBERING_COLUMN, number,
								PAYMENT_METHOD_TYPE_COLUMN, gsb_data_payment_get_sign (payment_number),
								PAYMENT_METHOD_DEFAULT_COLUMN, isdefault,
								PAYMENT_METHOD_ACTIVABLE_COLUMN, gsb_data_payment_get_sign (payment_number) != 0,
								PAYMENT_METHOD_VISIBLE_COLUMN, gsb_data_payment_get_sign (payment_number) != 0,
								PAYMENT_METHOD_NUMBER_COLUMN, payment_number,
								PAYMENT_METHOD_ACCOUNT_COLUMN, account_number,
								-1);
			g_free (number);

			payment_list = payment_list->next;
		}
		list_tmp = list_tmp->next;
	}
}

/**
 * Callback called when the payment method name is changed in the
 * GtkEntry associated.  It updates the GtkTreeView list of payment
 * methods as well as it updates transaction form.
 *
 * \param entry the entry changed (payment method name)
 * \param tree_view
 *
 * \return FALSE
 **/
static gboolean prefs_page_payment_method_name_changed (GtkWidget *entry,
                                                        GtkWidget *tree_view)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
		GtkTreeModel *model;
		gint payment_number;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
		gtk_tree_model_get (GTK_TREE_MODEL(model),
		                    &iter,
		                    PAYMENT_METHOD_NUMBER_COLUMN, &payment_number,
		                    -1);

		if (payment_number)
		{
			/* update the tree view */
			gtk_tree_store_set (GTK_TREE_STORE (model),
			                    &iter,
			                    PAYMENT_METHOD_NAME_COLUMN, gsb_data_payment_get_name (payment_number),
			                    -1);
			prefs_page_reconcile_sort_fill ();

			if (gsb_data_form_check_for_value (TRANSACTION_FORM_TYPE))
			{
				GtkWidget *widget;
				gint account_number;

				account_number = gsb_form_get_account_number ();
				widget = gsb_form_widget_get_widget (TRANSACTION_FORM_TYPE);

				if (widget)
				{
					gsb_payment_method_create_combo_list (widget,
														  GSB_PAYMENT_DEBIT,
														  account_number,
					                                      0,
					                                      FALSE);

					if (gtk_widget_get_visible (widget))
					{
						gsb_payment_method_set_combobox_history (widget,
						                                         gsb_data_account_get_default_debit
						                                         (account_number),
						                                         TRUE);
					}
					else
					{
					gtk_widget_hide (gsb_form_widget_get_widget (TRANSACTION_FORM_CHEQUE));
					}
			}
			}
			/* update the transactions list */
			transaction_list_update_element (ELEMENT_PAYMENT_TYPE);
		}
    }
    return FALSE;
}

/**
 * Callback called when the GtkSpinButton corresponding to the last
 * automatic number of the payment method is changed.  It updates the
 * list.
 *
 * \param spin_button
 * \param tree_view
 *
 * \return FALSE
 *
 **/
static void prefs_page_payment_method_number_changed (GtkWidget *spin_button,
                                                      GtkWidget *tree_view)
{
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
		GtkTreeModel *model;
		gint payment_number;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
		gtk_tree_model_get (GTK_TREE_MODEL(model),
		                    &iter,
		                    PAYMENT_METHOD_NUMBER_COLUMN, &payment_number,
		                    -1);
		if (payment_number)
		{
			const gchar* tmp_str;

			tmp_str = gsb_data_payment_get_last_number (payment_number);
			gtk_tree_store_set (GTK_TREE_STORE (model),
			                    &iter,
			                    PAYMENT_METHOD_NUMBERING_COLUMN, tmp_str,
			                    -1);
		}
    }
}

/**
 * This is a GtkTreeModelForeachFunc function.  It is used together
 * with gtk_tree_model_foreach() to search for `data' in the list.
 * Upon success, gsb_payment_method_config_foreach_select() also scrolls on current entry.
 *
 * \param model 			A pointer to the GtkTreeModel of the list.
 * \param path 				A GtkTreePath set to the current entry.
 * \param iter 				A GtkTreeIter set to the current entry.
 * \param payment_number	The payment number to match with current entry.
 *
 * \returns TRUE on success, which means gtk_tree_model_foreach() will
 *          stop browsing the tree.  FALSE otherwise.
 **/
static void prefs_page_payment_method_select_changed (GtkWidget *tree_view,
                                                      GtkTreeModel *model,
                                                      GtkTreePath *origin_path,
                                                      gint payment_number)
{
	if (origin_path)
	{
		GtkTreeIter iter;
		GtkTreeIter parent_iter;
		GtkTreeSelection *selection;
		gchar *str_path;
		gchar **tab_indices;
		GtkTreePath *path;
		GtkTreePath *tmp_path;

		str_path = gtk_tree_path_to_string (origin_path);
		tab_indices = g_strsplit (str_path, ":", 0);
		path = gtk_tree_path_new_from_string (tab_indices[0]);
		tmp_path = path;
		gtk_tree_model_get_iter (model, &parent_iter, path);

		if (gtk_tree_model_iter_children (model, &iter, &parent_iter))
		{
    		gint tmp_payment_number;

			do
			{
				GtkTreeIter child;

				if (gtk_tree_model_iter_children (model, &child, &iter))
				{
					do
					{
						if (!gtk_tree_store_iter_is_valid (GTK_TREE_STORE(model), &child))
							break;
						gtk_tree_model_get (GTK_TREE_MODEL(model),
											&child,
											PAYMENT_METHOD_NUMBER_COLUMN, &tmp_payment_number,
											-1);
						if (tmp_payment_number == payment_number)
						{
							path = gtk_tree_model_get_path (model, &iter);
							break;
						}
					} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &child));
				}
				else
				{
					gtk_tree_model_get (GTK_TREE_MODEL(model),
					                    &iter,
					                    PAYMENT_METHOD_NUMBER_COLUMN, &tmp_payment_number,
					                    -1);
					if (tmp_payment_number == payment_number)
					{
						path = gtk_tree_model_get_path (model, &iter);
						break;
					}
				}
		    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));

		}

		gtk_tree_view_expand_row (GTK_TREE_VIEW (tree_view), tmp_path, TRUE);
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, TRUE, 0.5, 0);

		g_strfreev (tab_indices);
		gtk_tree_path_free (path);
		gtk_tree_path_free (tmp_path);
	}
}

/**
 * callback called when changing the sign type of the method of payment
 *
 * \param combo 	the widget which receive the signal
 * \param page
 *
 * \return FALSE
 **/
static void prefs_page_payment_method_sign_changed (GtkWidget *combo,
                                                    PrefsPagePaymentMethod *page)
{
	GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean good;
	PrefsPagePaymentMethodPrivate *priv;

	priv = prefs_page_payment_method_get_instance_private (page);

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_payment));
    good = gtk_tree_selection_get_selected (selection, &model, &iter);

	if (good)
    {
		GtkTreePath *origin_path;
        gint index;
        gint payment_number;
		gint old_sign;

        index = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));
        gtk_tree_model_get (GTK_TREE_MODEL (model),
                            &iter,
                            PAYMENT_METHOD_NUMBER_COLUMN, &payment_number,
                            -1);

		if (payment_number)
        {
            gint account_number;

			old_sign = gsb_data_payment_get_sign (payment_number);
			if (old_sign == index)
			{
				return;
			}

			/* on récupère le path de l'iter de départ */
			origin_path = gtk_tree_model_get_path (model, &iter);

			/* on bloque l'appel à la fonction gsb_payment_method_config_sign_changed */
			g_signal_handlers_block_by_func (combo,
											 G_CALLBACK (prefs_page_payment_method_sign_changed),
											 page);

            account_number = gsb_data_payment_get_account_number (payment_number);

			/* as we have changed the sign of the method of payment, check if it was the default
             * for the account, and if yes, change the default for that account */
            switch (old_sign)
            {
				case GSB_PAYMENT_DEBIT:
					if (gsb_data_account_get_default_debit (account_number) == payment_number)
					{
						/* the current method of payment was a debit and was the default debit for its account,
						 * so change the default to another debit */
						gsb_data_account_set_default_debit (account_number,
						                                    gsb_data_payment_get_other_number_from_sign
						                                    (account_number, GSB_PAYMENT_DEBIT, payment_number));
					}
					break;

				case GSB_PAYMENT_CREDIT:
					if (gsb_data_account_get_default_credit (account_number) == payment_number)
					{
						/* the current method of payment was a credit and was the default credit for its account,
						 * so change the default to another credit */
						gsb_data_account_set_default_credit (account_number,
						                                     gsb_data_payment_get_other_number_from_sign
						                                     (account_number, GSB_PAYMENT_CREDIT, payment_number));
					}
					break;
            }
            gsb_data_payment_set_sign (payment_number, index);

            /* Update tree */
			g_signal_handlers_block_by_func (selection,
			                                 G_CALLBACK (prefs_page_payment_method_row_selected),
			                                 priv->treeview_payment);
            prefs_page_payment_method_fill (priv->treeview_payment);
			g_signal_handlers_unblock_by_func (selection,
											   G_CALLBACK (prefs_page_payment_method_row_selected),
											   priv->treeview_payment);
			/* sélectionne le nouveau moyen de paiement */
			prefs_page_payment_method_select_changed (priv->treeview_payment, model, origin_path, payment_number);

            /* need to clear and fill the reconciliation tree becaus if it was a neutral changing to credit/debit
             * and neutral was split... */
            prefs_page_reconcile_sort_fill ();

			/* on débloque l'appel à la fonction gsb_payment_method_config_sign_changed */
			g_signal_handlers_unblock_by_func (combo,
											   G_CALLBACK (prefs_page_payment_method_sign_changed),
											   page);
        }
    }
}

/**
 * callback called by the button to add a new method of payment
 *
 * \param button
 * \param tree_view the payment method config tree view
 *
 * \return FALSE
 **/
static void prefs_page_payment_method_add_clicked (GtkWidget *button,
                                                   GtkWidget *tree_view)
{
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
	GtkTreeIter child;
	GtkTreeIter *final;
	GtkTreeIter parent;
	GtkTreeIter root;
	GtkTreePath *final_path;
	GtkTreePath *tree_path;
	gint account_number;
	gint payment_number;
	gint type_final;
    gboolean good;

	devel_debug (NULL);
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    good = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (good)
    {
		/* there is a selection, get current iter */
        gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, PAYMENT_METHOD_NUMBER_COLUMN, &payment_number, -1);
        if (payment_number)
        {
            /* Select parent */
            gtk_tree_model_iter_parent (GTK_TREE_MODEL (model), &parent, &iter);
            final = &parent;
            type_final = gsb_data_payment_get_sign (payment_number);
            gtk_tree_model_iter_parent (GTK_TREE_MODEL (model), &root, &parent);
            /* ici root = compte, parent = final = type, iter = moyen de payement sélectionné */
        }
        else
        {
            gchar *name;

            gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, PAYMENT_METHOD_NAME_COLUMN, &name, -1);
            if (gtk_tree_model_iter_parent (GTK_TREE_MODEL (model), &root, &iter))
            {
                /* We are on "Credit" or "Debit" or "Neutral" */
                final = &iter;
                if (!strcmp (name, _("Credit")))
                {
                    type_final = GSB_PAYMENT_CREDIT;
                }
                else if (!strcmp (name, _("Debit")))
                {
                    type_final = GSB_PAYMENT_DEBIT;
                }
                else        /* Neutral */
                {
                    type_final = GSB_PAYMENT_NEUTRAL;
                }
            }
            else
            {
                /* We are on an account, method_ptr will be the same as the
                   first node  */
                if (!gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &child, &iter))
                    /* Should not happen! */
                    dialogue_error_brain_damage ();

                final = &child;
                type_final = GSB_PAYMENT_DEBIT;
            }
        }
    }
    else
    {
		/* No selection, we use first account, first method*/
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL(model), &iter)
			&& gtk_tree_model_iter_children (GTK_TREE_MODEL(model), &child, &iter))
		{
			final = &child;
			type_final = GSB_PAYMENT_DEBIT;
		}
		else
			return;
    }

    /* final is now set on debit or credit line where we want to set the method of payment */
    gtk_tree_model_get (GTK_TREE_MODEL(model), final, PAYMENT_METHOD_ACCOUNT_COLUMN, &account_number, -1);

    /* create the new method of payment */
    payment_number = gsb_data_payment_new (_("New payment method"));
    gsb_data_payment_set_sign (payment_number, type_final);
    gsb_data_payment_set_account_number (payment_number, account_number);

    /* append it to the store */
    gtk_tree_store_append (GTK_TREE_STORE (model), &iter, final);
    gtk_tree_store_set (GTK_TREE_STORE (model),
                        &iter,
            			PAYMENT_METHOD_NAME_COLUMN, gsb_data_payment_get_name (payment_number),
						PAYMENT_METHOD_NUMBERING_COLUMN, NULL,
						PAYMENT_METHOD_TYPE_COLUMN, type_final,
						PAYMENT_METHOD_DEFAULT_COLUMN, FALSE,
						PAYMENT_METHOD_ACTIVABLE_COLUMN, type_final != 0,
						PAYMENT_METHOD_VISIBLE_COLUMN, type_final != 0,
						PAYMENT_METHOD_NUMBER_COLUMN, payment_number,
						-1);

    tree_path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter);
    gtk_tree_model_row_inserted (GTK_TREE_MODEL (model), tree_path, &iter);

    /* Select and view new position */
    final_path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), final);
    if (!gtk_tree_view_row_expanded (GTK_TREE_VIEW (tree_view), final_path))
        gtk_tree_view_expand_to_path (GTK_TREE_VIEW (tree_view), final_path);
    gtk_tree_path_free (final_path);
    gtk_tree_selection_select_iter (selection, &iter);
    gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), tree_path, NULL, TRUE, 0.5, 0);
    gtk_tree_path_free (tree_path);

    /* add to the sorted list */
    gsb_data_account_sort_list_add (account_number, payment_number);
    prefs_page_reconcile_sort_fill ();

    /* Mark file as modified */
    utils_prefs_gsb_file_set_modified ();
}

/**
 * Callback called when automatic numbering is activated or
 * deactivated for current payment method.  It activates or
 * deactivates the "current number" field and updates display of
 * current number in the list.
 *
 * \param button the checkbutton which send the signal
 * \param tree_view
 *
 * \return FALSE
 **/
static void prefs_page_payment_method_button_auto_toggled (GtkToggleButton *button,
                                                           PrefsPagePaymentMethodPrivate *priv)
{
	GtkTreeModel *model;
    GtkTreeSelection *selection;
    gboolean good;
    GtkTreeIter iter;

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_payment));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
		gint payment_number;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_payment));
		gtk_tree_model_get (GTK_TREE_MODEL(model), &iter, PAYMENT_METHOD_NUMBER_COLUMN, &payment_number, -1);

		if (payment_number)
		{
			if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button)))
			{
				const gchar* tmp_str;

				gtk_widget_set_sensitive (priv->spinbutton_payment_number, TRUE);
				tmp_str = gsb_data_payment_get_last_number (payment_number);
				gtk_tree_store_set (GTK_TREE_STORE (model),
				                    &iter,
				                    PAYMENT_METHOD_NUMBERING_COLUMN, tmp_str,
				                    -1);
			}
			else
			{
				gtk_widget_set_sensitive (priv->spinbutton_payment_number, FALSE);
				gtk_tree_store_set (GTK_TREE_STORE (model),
									&iter,
									PAYMENT_METHOD_NUMBERING_COLUMN, NULL,
									-1);
			}
		}
    }
}

/**
 * callback called by a clicked signal on the removing button
 *
 * \param button
 * \param tree_view
 *
 * \return FALSE
 *
 **/
static void prefs_page_payment_method_remove_clicked (GtkWidget *button,
                                                      PrefsPagePaymentMethod *page)
{
    gint payment_number;
    gint account_number;
    GtkTreeSelection * selection;
    GtkTreeIter iter;
    gboolean good;
    GSList *tmp_list;
    GtkTreeModel *model;
	PrefsPagePaymentMethodPrivate *priv;

	priv = prefs_page_payment_method_get_instance_private (page);

	/** First, we find related GtkTreeIter and stsruct_type_ope pointer. */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_payment));
    good = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (!good)
		return;

    gtk_tree_model_get (GTK_TREE_MODEL(model),&iter, PAYMENT_METHOD_NUMBER_COLUMN, &payment_number, -1);
	if (!payment_number)
		return;

    /* we check if some transactions have this method of payment,
     * if yes, we propose to switch to another method of payment or cancel */

    /* first, check if there is some transactions */
	tmp_list = gsb_data_transaction_get_complete_transactions_list ();
	while (tmp_list)
	{
		TransactionStruct *transaction;

		transaction = tmp_list->data;
		if (transaction->method_of_payment_number == payment_number)
		{
			/* ok, there is some transactions with that method of payment, we switch to another one or cancel */
			if (!prefs_page_payment_method_switch_payment (page, payment_number))
				return;

			tmp_list = NULL;
		}
		else
			tmp_list = tmp_list->next;
	}

    account_number = gsb_data_payment_get_account_number (payment_number);

    /* If it was a default, change default */
	gsb_data_payment_change_default_payment_method (payment_number, account_number);

    /* remove that method of payment from the sort list */
    gsb_data_account_sort_list_remove (account_number, payment_number);

    /* Remove the method of payment from tree & memory */
    gtk_tree_store_remove (GTK_TREE_STORE(model), &iter);
    gsb_data_payment_remove (payment_number);

    utils_prefs_gsb_file_set_modified ();
}

/**
 * Callback called when show entry  is activated or
 * deactivated for current payment method.  It activates or
 * deactivates the automatic numbering button
 *
 * \param button the checkbutton which send the signal
 * \param priv
 *
 * \return FALSE
 */
static gboolean prefs_page_payment_method_show_entry_toggled (GtkWidget *checkbutton,
															  PrefsPagePaymentMethodPrivate *priv)
{
    GtkTreeSelection * selection;
    gboolean good;
    GtkTreeIter iter;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_payment));
    good = gtk_tree_selection_get_selected (selection, NULL, &iter);

    if (good)
    {
		GtkTreeModel *model;
		gint payment_number;

		model = gtk_tree_view_get_model (GTK_TREE_VIEW (priv->treeview_payment));
		gtk_tree_model_get (GTK_TREE_MODEL(model),
							&iter,
							PAYMENT_METHOD_NUMBER_COLUMN, &payment_number,
							-1);
		if (payment_number)
		{
			gboolean activ;

			activ = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbutton));
			if (activ)
				gtk_widget_set_sensitive (priv->checkbutton_payment_auto, TRUE);
			else
			{
				if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->checkbutton_payment_auto)))
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_payment_auto), FALSE);
				gtk_widget_set_sensitive (priv->checkbutton_payment_auto, FALSE);
			}
		}
    }
    return FALSE;
}

/**
 * Init the tree_view for the payment method list
 *
 * \param page
 *
 * \return
 **/
static void prefs_page_payment_method_setup_tree_view (PrefsPagePaymentMethod *page)
{
	GtkTreeStore *store = NULL;
	GtkCellRenderer *cell;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GtkTreeViewColumn *column;
	PrefsPagePaymentMethodPrivate *priv;

	priv = prefs_page_payment_method_get_instance_private (page);

	/* create the model */
	store = gtk_tree_store_new (NUM_PAYMENT_METHOD_COLUMNS,
								G_TYPE_STRING,			/* PAYMENT_METHOD_NAME_COLUMN */
								G_TYPE_STRING,			/* PAYMENT_METHOD_NUMBERING_COLUMN */
								G_TYPE_BOOLEAN,			/* PAYMENT_METHOD_DEFAULT_COLUMN */
								G_TYPE_INT,				/* PAYMENT_METHOD_TYPE_COLUMN */
								G_TYPE_BOOLEAN,			/* PAYMENT_METHOD_VISIBLE_COLUMN */
								G_TYPE_BOOLEAN,			/* PAYMENT_METHOD_ACTIVABLE_COLUMN */
								G_TYPE_INT,				/* PAYMENT_METHOD_NUMBER_COLUMN */
								G_TYPE_INT);			/* PAYMENT_METHOD_ACCOUNT_COLUMN */

	/* set tree_view model */
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->treeview_payment), GTK_TREE_MODEL (store));
	gtk_widget_set_name (priv->treeview_payment, "tree_view");
	g_object_unref (G_OBJECT (store));

	/* Account */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_pack_end (column, cell, TRUE);
	gtk_tree_view_column_set_title (column, _("Account"));
	gtk_tree_view_column_set_attributes (column,
										 cell,
										 "text", PAYMENT_METHOD_NAME_COLUMN,
										 NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_payment), column);

	/* Defaults */
	cell = gtk_cell_renderer_toggle_new ();
	gtk_cell_renderer_toggle_set_radio (GTK_CELL_RENDERER_TOGGLE(cell), TRUE);
	g_object_set (cell, "xalign", COLUMN_CENTER, NULL);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_alignment (column, COLUMN_CENTER);
	gtk_tree_view_column_pack_end (column, cell, TRUE);
	gtk_tree_view_column_set_title (column, _("Default"));
	gtk_tree_view_column_set_attributes (column,
										 cell,
										 "active", PAYMENT_METHOD_DEFAULT_COLUMN,
										 "activatable", PAYMENT_METHOD_ACTIVABLE_COLUMN,
										 "visible", PAYMENT_METHOD_VISIBLE_COLUMN,
										 NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_payment), column);
	g_signal_connect (cell,
					  "toggled",
					  G_CALLBACK (prefs_page_payment_method_cell_default_toggled),
					  priv->treeview_payment);

	/* Numbering */
	cell = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_pack_end (column, cell, TRUE);
	gtk_tree_view_column_set_title (column, _("Numbering"));
	gtk_tree_view_column_set_attributes (column,
										 cell,
										 "text", PAYMENT_METHOD_NUMBERING_COLUMN,
										 NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (priv->treeview_payment), column);

	/* fill the model */
	prefs_page_payment_method_fill (priv->treeview_payment);

	/* select the first item */
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->treeview_payment));
	path = gtk_tree_path_new_from_indices (0, -1);
	gtk_tree_selection_select_path (selection, path);
	gtk_tree_view_expand_row (GTK_TREE_VIEW (priv->treeview_payment), path, TRUE);
	gtk_tree_path_free (path);

	/* unsensitive details */
	prefs_page_payment_method_sensitive_widgets (priv, FALSE);

	/* set signals */
	g_signal_connect (selection,
					  "changed",
					  G_CALLBACK (prefs_page_payment_method_row_selected),
					  priv);
}

/**
 * Création de la page de gestion des payment_method
 *
 * \param page
 *
 * \return
 **/
static void prefs_page_payment_method_setup_page (PrefsPagePaymentMethod *page,
                                                  GrisbiPrefs *win)
{
	GtkWidget *head_page;
	PrefsPagePaymentMethodPrivate *priv;

	devel_debug (NULL);
	priv = prefs_page_payment_method_get_instance_private (page);

	/* On récupère le nom de la page */
	head_page = utils_prefs_head_page_new_with_title_and_icon (_("Payment methods"), "gsb-payment-32.png");
	gtk_box_pack_start (GTK_BOX (priv->vbox_payment_method), head_page, FALSE, FALSE, 0);
	gtk_box_reorder_child (GTK_BOX (priv->vbox_payment_method), head_page, 0);

	/* set the list of payment_method */
	prefs_page_payment_method_setup_tree_view (page);

    /* Payment method name */
    gsb_autofunc_entry_new_from_ui (priv->entry_payment_name,
                                    G_CALLBACK (prefs_page_payment_method_name_changed),
                                    priv->treeview_payment,
                                    G_CALLBACK (gsb_data_payment_set_name),
                                    0);

	/* Need entry field */
	gsb_autofunc_checkbutton_new_from_ui (priv->checkbutton_payment_entry,
										  FALSE,
										  G_CALLBACK (prefs_page_payment_method_show_entry_toggled),
										  priv,
										  G_CALLBACK (gsb_data_payment_set_show_entry),
										  0 );

    /* Automatic numbering */
    gsb_autofunc_spin_new_from_ui (priv->spinbutton_payment_number,
                                   0,
								   G_CALLBACK (prefs_page_payment_method_number_changed),
								   priv->treeview_payment,
								   G_CALLBACK (gsb_data_payment_set_last_number_from_int),
                        		   0);

    /* Activate numbering */
	gsb_autofunc_checkbutton_new_from_ui (priv->checkbutton_payment_auto,
										  FALSE,
										  G_CALLBACK (prefs_page_payment_method_button_auto_toggled),
										  priv,
										  G_CALLBACK (gsb_data_payment_set_automatic_numbering),
										  0);

	/* We set menu to "Neutral" as a default*/
	gtk_combo_box_set_active (GTK_COMBO_BOX (priv->combo_payment_type), 0);

	/* initialise la boite de dialogue switch payment method */
	prefs_page_payment_method_init_switch_dialog (page, win);

	/* set signal Payment method sign */
	g_signal_connect (G_OBJECT (priv->combo_payment_type),
					  "changed",
					  G_CALLBACK (prefs_page_payment_method_sign_changed),
					  page);

	/* set signal button collapse expand all */
	g_signal_connect (G_OBJECT (priv->button_payment_collapse),
					  "clicked",
					  G_CALLBACK (prefs_page_payment_method_button_collapse_expand_clicked),
					  page);

	/* Connect signal button Add button Remove*/
    g_signal_connect (G_OBJECT (priv->button_payment_add),
                      "clicked",
                      G_CALLBACK (prefs_page_payment_method_add_clicked),
                      priv->treeview_payment);

	g_signal_connect (priv->button_payment_remove,
					  "clicked",
					  G_CALLBACK (prefs_page_payment_method_remove_clicked),
					  page);

	/* Connect signal checkbutton payment auto*/
    g_signal_connect_after (G_OBJECT (priv->checkbutton_payment_auto),
                            "toggled",
                            G_CALLBACK (gsb_data_payment_set_last_number_from_int),
                            0);

}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void prefs_page_payment_method_init (PrefsPagePaymentMethod *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void prefs_page_payment_method_dispose (GObject *object)
{
	G_OBJECT_CLASS (prefs_page_payment_method_parent_class)->dispose (object);
}

static void prefs_page_payment_method_class_init (PrefsPagePaymentMethodClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = prefs_page_payment_method_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/prefs/prefs_page_payment_method.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  vbox_payment_method);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  button_payment_add);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  button_payment_collapse);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  button_payment_remove);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  checkbutton_payment_auto);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  checkbutton_payment_entry);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  combo_payment_type);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  entry_payment_name);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  spinbutton_payment_number);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  treeview_payment);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  button_collapse_all);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  button_expand_all);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
												  dialog_payment_switch);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
	                                              combo_dialog_payment);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
	                                              label_dialog_with_combo);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
	                                              label_dialog_without_combo);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass),
												  PrefsPagePaymentMethod,
	                                              box_dialog_with_combo);
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
PrefsPagePaymentMethod *prefs_page_payment_method_new (GrisbiPrefs *win)
{
	PrefsPagePaymentMethod *page;

	page = g_object_new (PREFS_PAGE_PAYMENT_METHOD_TYPE, NULL);
	prefs_page_payment_method_setup_page (page, win);

	return page;
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

