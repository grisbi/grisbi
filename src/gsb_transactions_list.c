/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2008-20020 Pierre Biava (grisbi@pierre.biava.name)                */
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

#include "config.h"

#include "include.h"
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "gsb_transactions_list.h"
#include "accueil.h"
#include "bet_data.h"
#include "custom_list.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_account.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_archive.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_form.h"
#include "gsb_data_fyear.h"
#include "gsb_data_import_rule.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_form_transaction.h"
#include "gsb_real.h"
#include "gsb_reconcile.h"
#include "gsb_scheduler_list.h"
#include "import.h"
#include "menu.h"
#include "mouse.h"
#include "navigation.h"
#include "print_transactions_list.h"
#include "structures.h"
#include "traitement_variables.h"
#include "transaction_list.h"
#include "transaction_list_select.h"
#include "transaction_list_sort.h"
#include "transaction_model.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_operations.h"
#include "utils_real.h"
#include "utils_str.h"
#include "widget_reconcile.h"
#include "widget_search_transaction.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *transactions_tree_view = NULL;	/* tree_view de la liste des opérations */
static gint current_tree_view_width = 0;			/* largeur courante du tree_view */

static GtkWidget *transaction_toolbar;	/* Barre d'outils */
static GtkWidget *menu_import_rules;	/* this button is showed or hidden if account have or no some rules */

/* the width of each column */
static gint transaction_col_width_init[CUSTOM_MODEL_VISIBLE_COLUMNS] = {10, 12, 30, 12, 12, 12, 12};	/* valeurs par défaut */
static gint transaction_col_width[CUSTOM_MODEL_VISIBLE_COLUMNS];

/* the alignment tab of each column */
static gint transaction_col_align_init[CUSTOM_MODEL_VISIBLE_COLUMNS] = { 1, 1, 0, 1, 2, 2, 2 };	/* valeurs par défaut */
static gint transaction_col_align[CUSTOM_MODEL_VISIBLE_COLUMNS];

/* contient les tips et titres des colonnes des listes d'opé */
static gchar *tips_col_liste_operations[CUSTOM_MODEL_VISIBLE_COLUMNS];
static gchar *titres_colonnes_liste_operations[CUSTOM_MODEL_VISIBLE_COLUMNS];

/* Liste des champs de la fenêtre d'affichage des tansactions */
static gint tab[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS] = {		/* Liste par défaut */
    {ELEMENT_CHQ, ELEMENT_DATE, ELEMENT_PARTY, ELEMENT_MARK, ELEMENT_DEBIT, ELEMENT_CREDIT, ELEMENT_BALANCE},
    {0, 0, ELEMENT_CATEGORY, 0, ELEMENT_PAYMENT_TYPE, ELEMENT_AMOUNT, 0},
    {0, 0, ELEMENT_NOTES, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0}
	};
static gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS];	/* Liste effective */

static const gchar *labels_titres_colonnes_liste_ope[] = {	/* names of the data for transactions list */
    N_("Date"),
    N_("Value date"),
    N_("Payee"),
    N_("Budgetary line"),
    N_("Debit"),
    N_("Credit"),
    N_("Balance"),
    N_("Amount"),
    N_("Method of payment"),
    N_("Reconciliation reference"),
    N_("Financial year"),
    N_("Category"),
    N_("C/R"),
    N_("Voucher"),
    N_("Notes"),
    N_("Bank references"),
    N_("Transaction number"),
    N_("Cheque number"),
    NULL };
/*END_STATIC*/

/*START_GLOBAL*/
/* the columns of the tree_view */
GtkTreeViewColumn *transactions_tree_view_columns[CUSTOM_MODEL_VISIBLE_COLUMNS];

/* this is used when opening a file and filling the model, */
/* when some children didn't find their mother */
GSList *orphan_child_transactions = NULL;
/*END_GLOBAL*/

/*START_EXTERN*/
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *  Check that a transaction is selected
 *
 * \param
 *
 * \return TRUE on success, FALSE otherwise.
 **/
static gboolean gsb_transactions_list_assert_selected_transaction (void)
{
    if (gsb_data_account_get_current_transaction_number (gsb_gui_navigation_get_current_account ()) == -1)
		return FALSE;

    return TRUE;
}

/**
 * Change the content of a cell.  This is triggered from the
 * activation of a menu item, so that we check the attributes of this
 * item to determine which cell is changed and with what.
 *
 * After this, iterate to update all GtkTreeIters of the transaction
 * list
 *
 * \param item		The GtkMenuItem that triggered event.
 * \param element	new element to put in the cell.
 *
 * \return FALSE
 **/
static void gsb_transactions_list_change_cell_content (GtkWidget *item,
													   gint *element_ptr)
{
    gint col, line;
    gint last_col = -1, last_line = -1;
    gint element;
    gint sort_column;
    gint current_account;

    element = GPOINTER_TO_INT (element_ptr);
    devel_debug_int (element);

    if (element)
    {
        last_col = gsb_transactions_list_find_element_col (element);
        last_line = gsb_transactions_list_find_element_line (element);
    }

    current_account = gsb_gui_navigation_get_current_account ();
    sort_column = gsb_data_account_get_sort_column (current_account);

    col = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), "x"));
    line = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), "y"));

    /* if no change, change nothing */
    if (last_col == col && last_line == line)
        return;

    /* save the new position */
    tab_affichage_ope[line][col] = element;

    if (last_col != -1 && last_line != -1)
    {
        /* the element was already showed, we need to erase the last cell first */
        tab_affichage_ope[last_line][last_col] = 0;
        transaction_list_update_cell (last_col, last_line);
    }

    /* now we can update the element */
    if (element)
        transaction_list_update_element (element);
    else
        transaction_list_update_cell (col, line);

    gsb_transactions_list_set_titles_tips_col_list_ope ();
    gsb_transactions_list_update_titres_tree_view ();

    /* update the sort column */
    gsb_data_account_set_element_sort (current_account, col, element);
    if (sort_column == last_col)
    {
        gsb_data_account_set_sort_column (current_account, col);
        transaction_list_sort_set_column (col, gsb_data_account_get_sort_type (current_account));
    }

    gsb_file_set_modified (TRUE);
}

/**
 * Create and return a menu that contains all cell content types.
 * When a type is selected, the cell that triggered this pop-up menu
 * is changed accordingly.
 *
 * \param x	Horizontal coordinate of the cell that will be modified.
 * \param y	Vertical coordinate of the cell that will be modified.
 *
 * \return	A newly-allocated menu.
 **/
static GtkWidget *gsb_transactions_list_create_cell_contents_menu (int x,
																   int y)
{
    GtkWidget * menu, * item;
    gint i;
    gint element_number;

    element_number = tab_affichage_ope[y][x];
    menu = gtk_menu_new ();

    for (i = 0 ; i < 18 ; i++)
    {
        gchar *tmp_str;

        tmp_str = gsb_transactions_list_get_column_title_from_element (i);
        item = gtk_check_menu_item_new_with_label (tmp_str);
        g_free (tmp_str);

        if (i == element_number -1)
            gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);

        g_object_set_data (G_OBJECT (item), "x", GINT_TO_POINTER (x));
        g_object_set_data (G_OBJECT (item), "y", GINT_TO_POINTER (y));
        g_signal_connect (G_OBJECT(item),
						  "activate",
						  G_CALLBACK (gsb_transactions_list_change_cell_content),
						  GINT_TO_POINTER (i+1));

        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    }
    /* set a menu to clear the cell except for the first line */
    if (y > 0)
    {
        item = gtk_menu_item_new_with_label (_("Clear cell"));

        g_object_set_data (G_OBJECT (item), "x", GINT_TO_POINTER (x));
        g_object_set_data (G_OBJECT (item), "y", GINT_TO_POINTER (y));
        g_signal_connect (G_OBJECT (item),
						  "activate",
						  G_CALLBACK (gsb_transactions_list_change_cell_content),
						  GINT_TO_POINTER (0));

        gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    }
    return menu;
}

/**
 * display contra_transaction
 *
 * \param
 *
 * \return
 **/
static void gsb_transactions_list_display_contra_transaction (gint *element_ptr)
{
    gint target_account;
    gint transaction_number;

    transaction_number = GPOINTER_TO_INT (element_ptr);
    target_account = gsb_data_transaction_get_account_number (transaction_number);

    if (gsb_gui_navigation_set_selection (GSB_ACCOUNT_PAGE, target_account, 0))
    {
        /* If transaction is reconciled, show reconciled transactions. */
        if (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
			&&
			!gsb_data_account_get_r (target_account))
        {
            gsb_transactions_list_mise_a_jour_affichage_r (TRUE);
        }

        transaction_list_select (transaction_number);
    }
}

/**
 * Move transaction to another account
 *
 * \param transaction_number Transaction to move to other account
 * \param target_account Account to move the transaction to
 *
 * return TRUE if ok
 **/
static gboolean gsb_transactions_list_move_transaction_to_account (gint transaction_number,
																   gint target_account)
{
    gint source_account;
    gint contra_transaction_number;
    gint current_account;
	GrisbiWinRun *w_run;

    devel_debug_int (target_account);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

    source_account = gsb_data_transaction_get_account_number (transaction_number);
    contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number);

    /* if it's a transfer, update the contra-transaction category line */
    if (contra_transaction_number > 0)
    {
        /* the transaction is a transfer, we check if the contra-transaction is not on
         * the target account */
        if (gsb_data_transaction_get_account_number (contra_transaction_number) == target_account)
        {
            dialogue_error (_("Cannot move a transfer on his contra-account"));
            return FALSE;
        }
    }

    /* we change now the account of the transaction */
    gsb_data_transaction_set_account_number (transaction_number, target_account);

    /* update the field of the contra transaction if necessary. Ce transfert ne doit pas
     * modifier la balance du compte */
    if (contra_transaction_number > 0)
        transaction_list_update_transaction (contra_transaction_number);


    /* normally we can change the account only by right click button
     * so the current transaction is selected,
     * so move the selection down */
    transaction_list_select_down (FALSE);

    /* normally we are on the source account, if ever we are not, check here
     * what we have to update */
    current_account = gsb_gui_navigation_get_current_account ();
    if (current_account == source_account || current_account == target_account)
        gsb_transactions_list_update_tree_view (current_account, FALSE);

    /* update the first page */
    w_run->mise_a_jour_liste_comptes_accueil = TRUE;
    w_run->mise_a_jour_soldes_minimaux = TRUE;

    return TRUE;
}

/**
 * Move selected transaction to another account.  Normally called as a
 * handler.
 *
 * \param menu_item The GtkMenuItem that triggered this handler.
 * \param
 *
 * \return FALSE
 **/
static gboolean gsb_transactions_list_move_transaction_to_account_from_sub_menu (GtkMenuItem *menu_item,
																  				 gpointer null)
{
    gint source_account;
    gint target_account;
	gint current_transaction;

    if (!gsb_transactions_list_assert_selected_transaction ())
		return FALSE;

    source_account = gsb_gui_navigation_get_current_account ();
    target_account = GPOINTER_TO_INT (g_object_get_data (G_OBJECT(menu_item), "account_number"));
	current_transaction = gsb_data_account_get_current_transaction_number (source_account);

    if (gsb_transactions_list_move_transaction_to_account (current_transaction, target_account))
    {
		gtk_notebook_set_current_page (GTK_NOTEBOOK (grisbi_win_get_notebook_general ()), 1);
		update_transaction_in_trees (gsb_data_account_get_current_transaction_number (source_account));
		gsb_data_account_colorize_current_balance (source_account);
		mise_a_jour_accueil (FALSE);
        gsb_file_set_modified (TRUE);
    }

    return FALSE;
}

/**
 * Pop up a menu with several actions to apply to current transaction.
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void gsb_transactions_list_popup_context_menu (gboolean full,
													  int x,
													  int y)
{
    GtkWidget *menu, *menu_item;
    gint transaction_number;
    gboolean mi_full = TRUE;
    gint contra_number;

    transaction_number = gsb_data_account_get_current_transaction_number (
                            gsb_gui_navigation_get_current_account ());
    /* Add a sub menu to display the contra transaction */
    contra_number = gsb_data_transaction_get_contra_transaction_number (transaction_number);

    /* full is used for the whites line, to unsensitive some fields in the menu */
    if (transaction_number < 0)
	full = FALSE;

    /* mi_full is used for children of transactions, to unselect some fields in the menu */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
	mi_full = FALSE;
    menu = gtk_menu_new ();

    if (contra_number > 0)
    {
        menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gsb-jump-16.png",
																	  _("Displays the contra-transaction")));
        g_signal_connect_swapped (G_OBJECT(menu_item),
                        "activate",
                        G_CALLBACK (gsb_transactions_list_display_contra_transaction),
                        GINT_TO_POINTER (contra_number));
        gtk_widget_set_sensitive (menu_item, full);
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

        /* Separator */
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());
    }

    /* Edit transaction */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gtk-edit-16.png", _("Edit transaction")));
    g_signal_connect_swapped (G_OBJECT(menu_item),
			       "activate",
			       G_CALLBACK (gsb_transactions_list_edit_transaction_by_pointer),
			       GINT_TO_POINTER (transaction_number));
    gtk_widget_set_sensitive (menu_item, full);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Separator */
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());

    /* New transaction */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label  ("gsb-new-transaction-16.png",
																   _("New transaction")));
    g_signal_connect (G_OBJECT(menu_item),
					  "activate",
					  G_CALLBACK (gsb_transactions_list_select_new_transaction),
					  NULL);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Delete transaction */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gtk-delete-16.png", _("Delete transaction")));
    g_signal_connect (G_OBJECT(menu_item), "activate", G_CALLBACK(gsb_transactions_list_remove_transaction), NULL);
    if (!full
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_TELEPOINTEE)
	gtk_widget_set_sensitive (menu_item, FALSE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* use transaction as template */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gtk-copy-16.png",
																  _("Use selected transaction as a template")));
    g_signal_connect (G_OBJECT(menu_item), "activate",
		       G_CALLBACK (gsb_transactions_list_clone_template), NULL);
    gtk_widget_set_sensitive (menu_item, full);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Clone transaction */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gtk-copy-16.png", _("Clone transaction")));
    g_signal_connect (G_OBJECT(menu_item), "activate",
		       G_CALLBACK (gsb_transactions_list_clone_selected_transaction), NULL);
    gtk_widget_set_sensitive (menu_item, full);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Separator */
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());

    /* Convert to scheduled transaction */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gsb-convert-16.png",
																  _("Convert transaction to scheduled transaction")));
    g_signal_connect (G_OBJECT(menu_item),
					  "activate",
					  G_CALLBACK(gsb_transactions_list_convert_transaction_to_sheduled),
					  NULL);
    gtk_widget_set_sensitive (menu_item, full && mi_full);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Move to another account */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gsb-jump-16.png",
																  _("Move transaction to another account")));
    if (!full
	 ||
	 !mi_full
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_TELEPOINTEE)
	gtk_widget_set_sensitive (menu_item, FALSE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Add accounts submenu */
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
							   GTK_WIDGET (gsb_account_create_menu_list
										   (G_CALLBACK (gsb_transactions_list_move_transaction_to_account_from_sub_menu),
											FALSE,
											FALSE)));


    /* Separator */
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());

	/* Searching */
	menu_item = GTK_WIDGET (utils_menu_item_new_from_resource_label ("gtk-search-16.png", _("Search")));
    g_signal_connect (G_OBJECT(menu_item),
					  "activate",
					  G_CALLBACK(gsb_transactions_list_search),
					  GINT_TO_POINTER (transaction_number));
    gtk_widget_set_sensitive (menu_item, full);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

	/* Separator */
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new());

    /* Change cell content. */
    menu_item = GTK_WIDGET (utils_menu_item_new_from_image_label ("gtk-properties-16.png", _("Change cell content")));
    if (full)
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
				    GTK_WIDGET (gsb_transactions_list_create_cell_contents_menu (x, y)));
    gtk_widget_set_sensitive (menu_item, full);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    /* Finish all. */
    gtk_widget_show_all (menu);

	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
}

/**
 * switch the mark of the transaction in the list between P or empty
 * it will mark/unmark the transaction and update the marked amount
 * if we are reconciling, update too the amounts of the reconcile panel
 *
 * \param transaction_number
 *
 * \return FALSE
 **/
static gboolean gsb_transactions_list_switch_mark (gint transaction_number)
{
    gint col;
    gint account_number;
	GrisbiWinRun *w_run;

    devel_debug_int (transaction_number);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* if no P/R column, cannot really mark/unmark the transaction... */
    col = gsb_transactions_list_find_element_col (ELEMENT_MARK);
    if (col == -1)
	    return FALSE;

    /* if we are on the white line, a R transaction or a child of split, do nothing */
    if (transaction_number == -1
	 ||
	 gsb_data_transaction_get_marked_transaction (transaction_number)== OPERATION_RAPPROCHEE
	 ||
	 gsb_data_transaction_get_mother_transaction_number (transaction_number))
	return FALSE;

    account_number = gsb_gui_navigation_get_current_account ();

    if (gsb_data_transaction_get_marked_transaction (transaction_number))
    {
        gsb_data_transaction_set_marked_transaction (transaction_number,
                                  OPERATION_NORMALE);
    }
    else
    {
        gsb_data_transaction_set_marked_transaction (transaction_number,
                                  OPERATION_POINTEE);
    }

    transaction_list_update_transaction (transaction_number);

    /* if it's a split, set the mark to the children */
    if (gsb_data_transaction_get_split_of_transaction (transaction_number))
    {
	GSList *list_tmp_transactions;
	gint mark;

	mark = gsb_data_transaction_get_marked_transaction (transaction_number);
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while (list_tmp_transactions)
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions->data);

	    if (gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
		gsb_data_transaction_set_marked_transaction (transaction_number_tmp,
							      mark);
	    list_tmp_transactions = list_tmp_transactions->next;
	}
    }

    /* if we are reconciling, update the amounts label */
	if (w_run->equilibrage)
	{
		/* pbiava 02/12/2009 : shows the balance after you mark the transaction */
		transaction_list_set_balances ();
		widget_reconcile_update_amounts ();
	}
    /* need to update the marked amount on the home page */
    gsb_gui_navigation_update_statement_label (account_number);
    w_run->mise_a_jour_liste_comptes_accueil = TRUE;

    gsb_file_set_modified (TRUE);

    return FALSE;
}

/**
 * called when press a mouse button on the transactions list
 *
 * \param tree_view
 * \param ev a GdkEventButton
  * \param
*
 * \return normally TRUE to block the signal, if we are outside the tree_view, FALSE
 **/
static gboolean gsb_transactions_list_button_press (GtkWidget *tree_view,
													GdkEventButton *ev,
													gpointer null)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeViewColumn *tree_column;
    gpointer transaction_pointer;
    gint transaction_number, column, line_in_transaction;
    gint what_is_line;
	GrisbiWinRun *w_run;

    /*     if we are not in the list, go away */
    if (ev->window != gtk_tree_view_get_bin_window (GTK_TREE_VIEW (tree_view)))
		return(FALSE);

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* first, give the focus to the list */
    gtk_widget_grab_focus (tree_view);

    /* get the path,
     * if it's a right button and we are not in the list, show the partial popup
     * else go away */
    if (!gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view),
										ev->x,
										ev->y,
										&path,
										&tree_column,
										NULL,
										NULL))
    {
		/* show the partial popup */
		if (ev->button == RIGHT_BUTTON)
			gsb_transactions_list_popup_context_menu (FALSE, -1, -1);

		return (TRUE);
    }

    /* ok we are on the list, get the transaction */
    model = GTK_TREE_MODEL (transaction_model_get_model());

    gtk_tree_model_get_iter (model, &iter, path);
    gtk_tree_model_get (model,
						&iter,
			 			CUSTOM_MODEL_TRANSACTION_ADDRESS, &transaction_pointer,
			 			CUSTOM_MODEL_TRANSACTION_LINE, &line_in_transaction,
			 			CUSTOM_MODEL_WHAT_IS_LINE, &what_is_line,
			 			-1);

    /* for now for an archive, check the double-click */
    if (what_is_line == IS_ARCHIVE)
    {
		if (ev->type == GDK_2BUTTON_PRESS)
		{
			gint archive_number;
			gint archive_store_number;
			const gchar *name;

			archive_store_number = gsb_data_archive_store_get_number (transaction_pointer);
			archive_number = gsb_data_archive_store_get_archive_number (archive_store_number);

			/* i don't know why but i had a crash with a NULL name of archive, so prevent here */
			name = gsb_data_archive_get_name (archive_number);
			if (name)
			{
				gchar* tmp_str;

				tmp_str = g_strdup_printf (_("Do you want to add the transactions of the archive %s "
												   "into the list?"),
												 name);
				if (dialogue_yes_no (tmp_str, NULL, GTK_RESPONSE_CANCEL))
					gsb_transactions_list_add_transactions_from_archive (archive_number,
																		 gsb_data_archive_store_get_account_number
																		 (archive_store_number),
																		 TRUE);

				g_free(tmp_str);
			}
			else
			{
				if (archive_number)
					warning_debug (_("An archive was clicked but Grisbi is unable to get the name. "
									 "It seems like a bug.\nPlease try to reproduce and contact the "
									 "Grisbi team."));
				else
					warning_debug (_("An archive was clicked but it seems to have the number 0, "
									 "which should not happen.\nPlease try to reproduce and contact "
									 "the Grisbi team."));
			}
		}
		return FALSE;
    }

    /* it's not an archive, so it's a transaction and select it */
    transaction_number = gsb_data_transaction_get_transaction_number (transaction_pointer);
    transaction_list_select (transaction_number);

    /* get the column */
    column = g_list_index (gtk_tree_view_get_columns (GTK_TREE_VIEW (tree_view)), tree_column);

    /*     if it's the right click, show the good popup */
    if (ev->button == RIGHT_BUTTON)
    {
		if (transaction_number == -1)
			gsb_transactions_list_popup_context_menu (FALSE, -1, -1);
		else
			gsb_transactions_list_popup_context_menu (TRUE, column, line_in_transaction);

		return(TRUE);
    }

    /*     check if we press on the mark */
    if (transaction_number != -1
		&& column == gsb_transactions_list_find_element_col (ELEMENT_MARK)
		&& ((w_run->equilibrage
			 && line_in_transaction == gsb_transactions_list_find_element_line (ELEMENT_MARK))
			|| ((ev->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)))
    {
		gsb_transactions_list_switch_mark (transaction_number);
		transaction_list_set_balances ();

		return TRUE;
    }

    /*  if double - click */
    if (ev->type == GDK_2BUTTON_PRESS)
		gsb_transactions_list_edit_transaction (transaction_number);

    /* have to return FALSE to continue the signal and open eventually
     * a child in the tree... but returning FALSE make the selection slower...
     * so check here x of mouse, if <16, so at left of list, perhaps it's
     * for open a child, so return FALSE, in other case return TRUE because
     * much much faster.*/
    if (ev->x <16)
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
static void gsb_transactions_list_change_alignment (GtkWidget *menu_item,
													gint *no_column)
{
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell_renderer;
    gint column_number;
    gint alignment;
    gfloat xalign = 0.0;

    if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menu_item)))
        return;

    column_number = GPOINTER_TO_INT (no_column);
    column = gtk_tree_view_get_column (GTK_TREE_VIEW (transactions_tree_view), column_number);
    cell_renderer = g_object_get_data (G_OBJECT (column), "cell_renderer");

    alignment = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (menu_item), "alignment"));
    switch (alignment)
    {
        case GTK_JUSTIFY_LEFT:
            xalign = COLUMN_LEFT;
            break;
        case GTK_JUSTIFY_CENTER:
            xalign = COLUMN_CENTER;
            break;
        case GTK_JUSTIFY_RIGHT:
            xalign = COLUMN_RIGHT;
            break;
		case GTK_JUSTIFY_FILL:
			break;
    }

	printf ("alignment = %d xalign = %f\n", alignment, xalign);
    transaction_col_align[column_number] = alignment;
    gtk_tree_view_column_set_alignment  (column, xalign);
    g_object_set (G_OBJECT (cell_renderer), "xalign", xalign, NULL);

    gsb_file_set_modified (TRUE);
}

/**
 * Renvoie un élément de tri valide si celui passé en paramètre n'est pas une donnée
 * valide de la colonne
 *
 * \param account_number
 * \param column_number
 * \param element_number élement à tester
 *
 * \return old element or element 0 si non trouvé
 **/
static gint gsb_transactions_list_get_valid_element_sort (gint account_number,
													      gint column_number,
													      gint element_number)
{
    gint i;

    for (i = 0 ; i < 4 ; i++)
    {
        if (tab_affichage_ope[i][column_number] == element_number)
            return element_number;
    }

    return tab_affichage_ope[0][column_number];
}

/**
 * called by a click on a column title ; the sort of the list is automatic,
 * that function make the background color and the rest to be updated
 *
 * \param tree_view_column the tree_view_column clicked, not used, can be NULL
 * \param column_ptr	the number of column stocked as gint *, use GPOINTER_TO_INT
 *
 * \return FALSE
 **/
static gboolean gsb_transactions_list_change_sort_column (GtkTreeViewColumn *tree_view_column,
														  gint *column_ptr)
{
    GtkSortType sort_type = -1;
    gint current_column = -1;
    gint account_number;
    gint new_column;
    GSList *tmp_list;
    gint selected_transaction;
    gint element_number;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;

    devel_debug (NULL);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

    account_number = gsb_gui_navigation_get_current_account ();
    transaction_list_sort_get_column (&current_column, &sort_type);
    new_column = GPOINTER_TO_INT (column_ptr);

    element_number = gsb_data_account_get_element_sort (account_number, new_column);

    if (element_number == ELEMENT_BALANCE || element_number == 0)
        return FALSE;

    /* if we come here and the list was user custom sorted for reconcile,
     * we stop the reconcile sort and set what is asked by the user */
    if (transaction_list_sort_get_reconcile_sort ())
    {
        transaction_list_sort_set_reconcile_sort (FALSE);
		widget_reconcile_set_active_sort_checkbutton (FALSE);
        /* if we asked the same last column, we invert the value,
         * to come back to the last sort_type before the sort reconciliation */
        if (new_column == current_column)
        {
            if (sort_type == GTK_SORT_ASCENDING)
                sort_type = GTK_SORT_DESCENDING;
            else
                sort_type = GTK_SORT_ASCENDING;
        }
    }

    /* if the new column is the same as the old one, we change the sort type */
    if (new_column == current_column)
    {
        if (sort_type == GTK_SORT_ASCENDING)
            sort_type = GTK_SORT_DESCENDING;
        else
            sort_type = GTK_SORT_ASCENDING;
    }
    else
    {
        gint new_element;

        /* on vérifie que l'élément de tri existe sinon on met le premier élément de la colonne */
        new_element = gsb_transactions_list_get_valid_element_sort (account_number,
																	new_column,
																	element_number);
        if (new_element != element_number)
        {
            gsb_data_account_set_element_sort (account_number, new_column, new_element);
        }
        /* we sort by another column, so sort type by default is descending */
        sort_type = GTK_SORT_ASCENDING;
    }

    /* now have to save the new column and sort type in the account
     * or in all account if global conf for all accounts */
    tmp_list = gsb_data_account_get_list_accounts ();
    while (tmp_list)
    {
        gint tmp_account;

        tmp_account = gsb_data_account_get_no_account (tmp_list->data);

        if (tmp_account == account_number
            ||
            !w_etat->retient_affichage_par_compte)
        {
            /* set the new column to sort */
            gsb_data_account_set_sort_column (tmp_account, new_column);
            /* save the sort_type */
            gsb_data_account_set_sort_type (tmp_account, sort_type);
        }
        tmp_list = tmp_list->next;
    }

    selected_transaction = transaction_list_select_get ();

    /* now we can sort the list */
    transaction_list_sort_set_column (new_column, sort_type);
    transaction_list_filter (account_number);
    transaction_list_set_balances ();
    transaction_list_sort ();
    transaction_list_colorize ();
	if (a_conf->show_transaction_gives_balance)
		transaction_list_set_color_jour (account_number);
	transaction_list_select (selected_transaction);

    gsb_file_set_modified (TRUE);

	return FALSE;
}

/**
 * called when choose a new element to sort the list (from the popup of a right click on
 * the title of columns)
 *
 * \param menu_item The GtkMenuItem
 * \param no_column a pointer containing the number of the column we change
 *
 * \return FALSE
 **/
static void gsb_transactions_list_change_sort_type (GtkWidget *menu_item,
													gint *no_column)
{
    gint column_number;
    gint account_number;

    devel_debug_int (GPOINTER_TO_INT (no_column));

    if (!gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menu_item)))
		return;

    column_number = GPOINTER_TO_INT (no_column);
    account_number = gsb_gui_navigation_get_current_account ();

    /* set the new column to sort */
    gsb_data_account_set_sort_column (account_number, column_number);

    /* set the new element number used to sort this column */
    gsb_data_account_set_element_sort (account_number,
									   column_number,
									   GPOINTER_TO_INT (g_object_get_data (G_OBJECT (menu_item), "no_sort")));

    /* we want a descending sort but gsb_transactions_list_change_sort_column will
     * invert the order, so set DESCENDING for now */
    transaction_list_sort_set_column (column_number, GTK_SORT_DESCENDING);
    gsb_transactions_list_change_sort_column (NULL, no_column);
}

/**
 * Check if the transaction (or linked transactions) is not marked as Reconciled
 *
 * \param transaction_number
 *
 * \return TRUE : it's marked R ; FALSE : it's normal, P or T transaction
 **/
static gboolean gsb_transactions_list_check_mark (gint transaction_number)
{
    gint contra_transaction_number;

    /* vérifications de bases */
    if (transaction_number <= 0)
		return FALSE;

    if (gsb_data_transaction_get_marked_transaction (transaction_number)== OPERATION_RAPPROCHEE)
		return TRUE;

    /* if it's a transfer, check the contra-transaction */
    contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number);
    if (contra_transaction_number > 0)
    {
		if (gsb_data_transaction_get_marked_transaction (contra_transaction_number) == OPERATION_RAPPROCHEE)
			return TRUE;
    }

    /* if it's a split of transaction, check all the children
     * if there is not a transfer which is marked */
    if (gsb_data_transaction_get_split_of_transaction (transaction_number))
    {
		GSList *list_tmp_transactions;
		list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

		while (list_tmp_transactions)
		{
			gint transaction_number_tmp;

			transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions->data);

			if (gsb_data_transaction_get_account_number (transaction_number_tmp) ==
				gsb_data_transaction_get_account_number (transaction_number)
				&&
				gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
			{
				/* transactions_tmp is a child of transaction */
				if (gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_RAPPROCHEE)
					return TRUE;

				contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp);

				if (contra_transaction_number > 0)
				{
					/* the split is a transfer, we check the contra-transaction */
					if (gsb_data_transaction_get_marked_transaction (contra_transaction_number)== OPERATION_RAPPROCHEE)
					return TRUE;
				}
			}
			list_tmp_transactions = list_tmp_transactions->next;
		}
    }

	return FALSE;
}

/**
 * create a popup with the list of the reconciliations for the given account
 * to choose one
 *
 * \param account_number
 * \param transaction_number, we will select that reconciliation in the list
 *
 * \return the number of the chosen reconciliation or 0 if cancel
 **/
static gint gsb_transactions_list_choose_reconcile (gint account_number,
											 		gint transaction_number)
{
    GtkWidget *dialog;
	GtkWidget *button_cancel;
	GtkWidget *button_OK;
	GtkWidget *content_area;
    GtkWidget *label;
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    GList *tmp_list;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
	const gchar *titles[] = {N_("Name"), N_("Init date"), N_("Final date")};
    gint return_value;
    gint i;
    gint reconcile_number;
	gfloat alignment[] = {COLUMN_LEFT, COLUMN_CENTER, COLUMN_CENTER};
    enum reconcile_choose_column {
	RECONCILE_CHOOSE_NAME = 0,
	RECONCILE_CHOOSE_INIT_DATE,
	RECONCILE_CHOOSE_FINAL_DATE,
	RECONCILE_NUMBER,
	RECONCILE_NB_COL
	};

    dialog = gtk_dialog_new_with_buttons (_("Selection of a reconciliation"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_DIALOG_MODAL,
										  NULL, NULL,
										  NULL);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_cancel, GTK_RESPONSE_CANCEL);
	gtk_widget_set_can_default (button_cancel, TRUE);

	button_OK = gtk_button_new_with_label (_("Validate"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_OK, GTK_RESPONSE_OK);
	gtk_widget_set_can_default (button_OK, TRUE);

	gtk_window_set_default_size (GTK_WINDOW (dialog), 770, 412);
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (dialog), BOX_BORDER_WIDTH);

	content_area = gtk_dialog_get_content_area GTK_DIALOG (dialog);

	label = gtk_label_new (_("Select the reconciliation to associate to the selected transaction: "));
    utils_labels_set_alignment (GTK_LABEL (label), 0.0, 0.0);
    gtk_box_pack_start (GTK_BOX (content_area), label, FALSE, FALSE, MARGIN_BOX);

    scrolled_window = gtk_scrolled_window_new (FALSE, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start (GTK_BOX (content_area), scrolled_window, TRUE, TRUE, 0);

    /* set up the tree view */
    store = gtk_list_store_new (RECONCILE_NB_COL,
								G_TYPE_STRING,		/* RECONCILE_CHOOSE_NAME */
								G_TYPE_STRING,		/* CHOOSE_INIT_DATE */
								G_TYPE_STRING,		/* RECONCILE_CHOOSE_FINAL_DATE */
								G_TYPE_INT);		/* RECONCILE_NUMBER */

	tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
	gtk_widget_set_name (tree_view, "tree_view");
    g_object_unref (G_OBJECT(store));

    gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);

    /* set the columns */
    for (i = RECONCILE_CHOOSE_NAME ; i<RECONCILE_NUMBER ; i++)
    {
		GtkTreeViewColumn *column;
		GtkCellRenderer *cell;

		cell = gtk_cell_renderer_text_new ();
		g_object_set (G_OBJECT (cell), "xalign", alignment[i], NULL);
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		gtk_tree_view_column_set_alignment (column, alignment[i]);
		gtk_tree_view_column_pack_start (column, cell, TRUE);
		gtk_tree_view_column_set_title (column, gettext (titles[i]));
		gtk_tree_view_column_set_attributes (column, cell, "text", i, NULL);
		gtk_tree_view_column_set_expand (column, TRUE);
		gtk_tree_view_column_set_resizable (column, TRUE);
		gtk_tree_view_append_column (GTK_TREE_VIEW(tree_view), column);
    }

    /* get the tree view selection here to select the good reconcile */
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
    gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);

    /* fill the list */
    tmp_list = gsb_data_reconcile_get_reconcile_list ();

    while (tmp_list)
    {
        reconcile_number = gsb_data_reconcile_get_no_reconcile (tmp_list->data);

        if (gsb_data_reconcile_get_account (reconcile_number) == account_number)
        {
            GtkTreePath *path;
            gchar *init_date, *final_date;
            gint tmp_reconcile_number;

            init_date = gsb_format_gdate (gsb_data_reconcile_get_init_date (reconcile_number));
            final_date = gsb_format_gdate (gsb_data_reconcile_get_final_date (reconcile_number));

            gtk_list_store_append (GTK_LIST_STORE (store), &iter);
            gtk_list_store_set (GTK_LIST_STORE (store),
								&iter,
                        		RECONCILE_CHOOSE_NAME, gsb_data_reconcile_get_name (reconcile_number),
                        		RECONCILE_CHOOSE_INIT_DATE, init_date,
                        		RECONCILE_CHOOSE_FINAL_DATE, final_date,
                        		RECONCILE_NUMBER, reconcile_number,
                        		-1);

            /* select the reconcile here */
            tmp_reconcile_number = gsb_data_transaction_get_reconcile_number (transaction_number);
            if (tmp_reconcile_number == 0)
            {
                GDate *date_debut;
                GDate *date_fin;
                const GDate *date;

                date_debut = gsb_parse_date_string (init_date);
                date_fin = gsb_parse_date_string (final_date);
                date = gsb_data_transaction_get_date (transaction_number);
                if (g_date_compare (date, date_debut) >= 0
					&&
					g_date_compare (date, date_fin) <= 0)
                {
                    path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &iter);
                    gtk_tree_selection_select_iter (selection, &iter);
                    gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, FALSE, 0.0, 0.0);

                    gtk_tree_path_free (path);
                }
                g_date_free (date_debut);
                g_date_free (date_fin);
            }
            else if (tmp_reconcile_number == reconcile_number)
            {
                path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &iter);
                gtk_tree_selection_select_iter (selection, &iter);
                gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree_view), path, NULL, FALSE, 0.0, 0.0);
            }

            g_free (init_date);
            g_free (final_date);

        }
        tmp_list = tmp_list->next;
    }

	gtk_widget_show_all (dialog);

    /* run the dialog */
    return_value = gtk_dialog_run (GTK_DIALOG (dialog));

    if (return_value != GTK_RESPONSE_OK)
    {
		gtk_widget_destroy (dialog);

		return 0;
    }

    if (gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), NULL, &iter))
    {
		/* ok, we have a selection */

		gtk_tree_model_get (GTK_TREE_MODEL (store),
							&iter,
							RECONCILE_NUMBER, &reconcile_number,
							-1);
		gtk_widget_destroy (dialog);

		return reconcile_number;
    }

    dialogue_error (_("Grisbi couldn't get the selection, operation canceled..."));
    gtk_widget_destroy (dialog);

	return 0;
}

/**
 * Clone transaction.  If it is a split or a transfer, perform all
 * needed operations, like cloning associated transactions as well.
 *
 * \param transaction_number 		Initial transaction to clone
 * \param mother_transaction_number	if the transaction cloned is a child with another mother, this is the new mother
 *
 * \return the number newly created transaction.
 **/
static gint gsb_transactions_list_clone_transaction (gint transaction_number,
											  		 gint mother_transaction_number)
{
	gint account_number;
    gint new_transaction_number;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* dupplicate the transaction */
	account_number = gsb_data_transaction_get_account_number (transaction_number);
    new_transaction_number = gsb_data_transaction_new_transaction (account_number);
    gsb_data_transaction_copy_transaction (transaction_number, new_transaction_number, TRUE);

    if (gsb_data_transaction_get_mother_transaction_number (transaction_number)
		&&
		mother_transaction_number)
		gsb_data_transaction_set_mother_transaction_number (new_transaction_number,
															mother_transaction_number);

    /* create the contra-transaction if necessary */
    if (gsb_data_transaction_get_contra_transaction_number (transaction_number) > 0)
    {
		gsb_form_transaction_validate_transfer (new_transaction_number,
												1,
												gsb_data_transaction_get_contra_transaction_account
												(transaction_number));

		/* we need to set the contra method of payment of the transfer */
		gsb_data_transaction_set_method_of_payment_number (gsb_data_transaction_get_contra_transaction_number
														   (new_transaction_number),
														   gsb_data_transaction_get_method_of_payment_number
														   (gsb_data_transaction_get_contra_transaction_number
															(transaction_number)));
    }

    gsb_transactions_list_append_new_transaction (new_transaction_number, TRUE);

    if (gsb_data_transaction_get_split_of_transaction (transaction_number))
    {  /* the transaction was a split, we look for the children to copy them */
        GSList *list_tmp_transactions;

		list_tmp_transactions = gsb_data_transaction_get_transactions_list ();
        while (list_tmp_transactions)
        {
            gint transaction_number_tmp;
            transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions->data);

            if (gsb_data_transaction_get_account_number (transaction_number_tmp)
				== gsb_data_transaction_get_account_number (transaction_number)
				&&
				gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp)
				== transaction_number)
            	gsb_transactions_list_clone_transaction (transaction_number_tmp, new_transaction_number);

            list_tmp_transactions = list_tmp_transactions->next;
        }
    }
    if (w_run->equilibrage)
        transaction_list_show_toggle_mark (TRUE);

    return new_transaction_number;
}

/**
 * creates the columns of the tree_view
 *
 * \param
 *
 * \return
 **/
static void gsb_transactions_list_create_tree_view_columns (void)
{
    gint i;
    gint column_balance;

    /* get the position of the amount column to set it in red */
    column_balance = gsb_transactions_list_find_element_col (ELEMENT_BALANCE);

    /* create the columns */
    for (i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++)
    {
		GtkCellRenderer *cell_renderer;
		gfloat xalign = 0.0;

		cell_renderer = gtk_cell_renderer_text_new ();

		/* fix bug 2181 */
	    switch (transaction_col_align[i])
		{
			case GTK_JUSTIFY_LEFT:
				xalign = COLUMN_LEFT;
				break;
			case GTK_JUSTIFY_CENTER:
				xalign = COLUMN_CENTER;
				break;
			case GTK_JUSTIFY_RIGHT:
				xalign = COLUMN_RIGHT;
				break;
			case GTK_JUSTIFY_FILL:
				break;
		}
		g_object_set (G_OBJECT (cell_renderer), "xalign", xalign, NULL);

		transactions_tree_view_columns[i] = gtk_tree_view_column_new_with_attributes
			(_(titres_colonnes_liste_operations[i]),
			 cell_renderer,
			 "text", i,
			 "cell-background-rgba", CUSTOM_MODEL_BACKGROUND,
			 "font", CUSTOM_MODEL_FONT,
			 NULL);
		g_object_set_data (G_OBJECT (transactions_tree_view_columns[i]), "cell_renderer", cell_renderer);

		if (i == column_balance)
			gtk_tree_view_column_add_attribute (transactions_tree_view_columns[i],
												cell_renderer,
												"foreground", CUSTOM_MODEL_AMOUNT_COLOR);
		else
			gtk_tree_view_column_add_attribute (transactions_tree_view_columns[i],
												cell_renderer,
												"foreground-rgba", CUSTOM_MODEL_TEXT_COLOR);

		if (i == gsb_transactions_list_find_element_col (ELEMENT_MARK))
		{
			GtkCellRenderer *radio_renderer;

			radio_renderer = gtk_cell_renderer_toggle_new ();
			gtk_tree_view_column_pack_start (transactions_tree_view_columns[i],
											 radio_renderer,
											 FALSE);
			gtk_tree_view_column_set_attributes (transactions_tree_view_columns[i],
												 radio_renderer,
												 "active", CUSTOM_MODEL_CHECKBOX_ACTIVE,
												 "activatable", CUSTOM_MODEL_CHECKBOX_VISIBLE,
												 "visible", CUSTOM_MODEL_CHECKBOX_VISIBLE,
												 "cell-background-rgba", CUSTOM_MODEL_BACKGROUND,
												 NULL);
			g_object_set_data (G_OBJECT (transactions_tree_view_columns[i]), "radio_renderer", radio_renderer);
		}

		if (i == CUSTOM_MODEL_VISIBLE_COLUMNS -1)
		{
			gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (cell_renderer), MARGIN_BOX, 0);
		}

		gtk_tree_view_column_set_alignment (transactions_tree_view_columns[i], xalign);
		gtk_tree_view_column_set_sizing (transactions_tree_view_columns[i], GTK_TREE_VIEW_COLUMN_FIXED);
		gtk_tree_view_column_set_resizable (transactions_tree_view_columns[i], TRUE);
    }
}

/**
 * switch the mark of the transaction in the list between R or empty
 * it will mark/unmark the transaction and update the marked amount
 * when we mark, we show a list of reconciliation and try to find the last reconciliation used by that
 * 	transaction if it exists, from the 0.6.0, a transaction marked R shouldn't be without
 * 	reconcile number (but if come from before, it could happen)
 * when we unmark, we keep the reconcile number into the transaction to find it easily when the user
 * 	will re-R again
 *
 * \param transaction_number
 *
 * \return FALSE
 **/
static gboolean gsb_transactions_list_switch_R_mark (gint transaction_number)
{
    gint account_number;
    GtkTreeIter iter;
    gint r_column;
    gchar *tmp_str1;
    gchar *tmp_str2;
	GrisbiWinRun *w_run;

	r_column = gsb_transactions_list_find_element_col (ELEMENT_MARK);
	if (r_column == -1)
		return FALSE;

	/* if we are on the white line or a child of split, do nothing */
	if (transaction_number == -1)
		return FALSE;

    /* if we are reconciling, cancel the action */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	if (w_run->equilibrage)
	{
		dialogue_error (_("You cannot switch a transaction between R and non R "
						  "while reconciling.\nPlease finish or cancel the "
						  "reconciliation first."));
		return FALSE;
	}

    /* if it's a child, we ask if we want to work with the mother */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
		if (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE)
		{
			tmp_str1 = g_strdup (_("You are trying to unreconcile a transaction manually, "
								   "which is not a recommended action. This is the wrong approach.\n\n"
								   "And moreover the transaction you try to reconcile is a child of split, so "
								   "the modification will be done on the mother and all its children.\n\n"
								   "Are you really sure to know what you do?"));
			tmp_str2 = g_strdup ("unreconcile-transaction");
		}
		else
		{
			tmp_str1 = g_strdup (_("You are trying to reconcile a transaction manually, "
								   "which is not a recommended action. This is the wrong approach.\n\n"
								   "And moreover the transaction you try to reconcile is a child of split, so "
								   "the modification will be done on the mother and all its children.\n\n"
								   "Are you really sure to know what you do?"));
			tmp_str2 = g_strdup ("reconcile-transaction");
		}

		if (dialogue_conditional_yes_no_with_items ("tab_warning_msg", tmp_str2, tmp_str1))
        {
            /* he says ok, so transaction_number becomes the mother */
            transaction_number = gsb_data_transaction_get_mother_transaction_number (transaction_number);
			g_free (tmp_str1);
			g_free (tmp_str2);
        }
	    else
        {
			g_free (tmp_str1);
			g_free (tmp_str2);

			return FALSE;
        }
    }
    else
	{
	    /* it's a normal transaction, ask to be sure */
		if (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE)
			tmp_str2 = g_strdup ("unreconcile-transaction");
		else
			tmp_str2 = g_strdup ("reconcile-transaction");

        if (!dialogue_conditional_yes_no (tmp_str2))
		{
            g_free (tmp_str2);

			return FALSE;
		}
		g_free (tmp_str2);
	}

    if (!transaction_model_get_transaction_iter (&iter,
						  transaction_number, 0))
	return FALSE;

    account_number = gsb_gui_navigation_get_current_account ();

    if (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE)
    {
	/* ok, this is a R transaction, we just un-R it but keep the reconcile_number into the transaction */
	gsb_data_transaction_set_marked_transaction (transaction_number,
						      OPERATION_NORMALE);
	transaction_list_set (&iter, r_column, NULL, -1);
    }
    else
    {
	/* this is a non R transaction we want to mark R
	 * we show a list of possible reconciliation to the user ; he must
	 * associate the transaction with a reconciliation */
	gint reconcile_number;

	reconcile_number = gsb_transactions_list_choose_reconcile (account_number,transaction_number);
	if (!reconcile_number)
	    return FALSE;

	gsb_data_transaction_set_marked_transaction (transaction_number,
						      OPERATION_RAPPROCHEE);
	gsb_data_transaction_set_reconcile_number (transaction_number,
						    reconcile_number);

	/* set the R on the transaction */
	transaction_list_set (&iter, r_column, g_strdup (_("R")), -1);

	/* if we don't want to see the marked R transactions, we re-filter the model */
	if (!gsb_data_account_get_r (account_number))
	{
	    transaction_list_select_down (FALSE);
	    gsb_data_account_set_current_transaction_number (account_number,
							      transaction_list_select_get ());
	    gsb_transactions_list_update_tree_view (account_number, TRUE);

	    /* we warn the user the transaction disappear
	     * don't laugh !there were several bugs reports about a transaction which disappear :-) */
	    dialogue_hint (_("The transaction has disappear from the list...\nDon't worry, it's because you marked it as R, and you choosed not to show the R transactions into the list ; show them if you want to check what you did."),
			   _("Marking a transaction as R"));
	}
    }

    /* if it's a split, set the mark to the children */
    if (gsb_data_transaction_get_split_of_transaction (transaction_number))
    {
	GSList *list_tmp_transactions;
	gint mark;

	mark = gsb_data_transaction_get_marked_transaction (transaction_number);
	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

	while (list_tmp_transactions)
	{
	    gint transaction_number_tmp;
	    transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions->data);

	    if (gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number
		 &&
		 gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
		gsb_data_transaction_set_marked_transaction (transaction_number_tmp,
							      mark);

	    list_tmp_transactions = list_tmp_transactions->next;
	}
        transaction_list_update_element (ELEMENT_MARK);
    }
    /* need to update the marked amount on the home page */
    w_run->mise_a_jour_liste_comptes_accueil = TRUE;

    gsb_file_set_modified (TRUE);

    return FALSE;
}

/**
 * called when press a button on the title column
 * check the right click and show a popup to chose the element number of
 * the column to sort the list
 *
 * \param button
 * \param ev
 * \param no_column a pointer which is the number of the column the user press
 *
 * \return FALSE
 **/
static gboolean gsb_transactions_list_title_column_button_press (GtkWidget *button,
																 GdkEventButton *ev,
																 gint *no_column)
{
    GtkWidget *menu = NULL;
    GtkWidget *menu_item = NULL;
    gint i;
    gint active_sort;
    gint column_number;

    column_number = GPOINTER_TO_INT (no_column);

    if (ev->button == 3)
    {
		/* we press the right button, show the popup */
		active_sort = gsb_data_account_get_element_sort (gsb_gui_navigation_get_current_account (),
														 column_number);

		/* get the name of the labels of the columns and put them in a menu */
		for (i=0 ; i<4 ; i++)
		{
			gchar *temp;

			switch (tab_affichage_ope[i][column_number])
			{
				case 0:
				temp = NULL;
				break;

				default:
				temp = gsb_transactions_list_get_column_title (i,column_number);
			}

			if (temp)
			{
				if (strcmp (temp, _("Balance")) == 0)
				{
					gsb_data_account_set_element_sort (gsb_gui_navigation_get_current_account (),
													   column_number,
													   ELEMENT_BALANCE);
					menu = gtk_menu_new ();

					/* Colonne balance on sort de la boucle */
					break;
				}
				else
				{
					if (menu == NULL)
					{
						menu = gtk_menu_new ();

						/* sort by line */
						menu_item = gtk_menu_item_new_with_label (_("Sort list by: "));
						gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
						gtk_widget_show (menu_item);

						menu_item = gtk_separator_menu_item_new ();
						gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
						gtk_widget_show (menu_item);
					}

					if (i && GTK_IS_RADIO_MENU_ITEM (menu_item))
						menu_item = gtk_radio_menu_item_new_with_label_from_widget (GTK_RADIO_MENU_ITEM
																					(menu_item),
																					temp);
					else
						menu_item = gtk_radio_menu_item_new_with_label (NULL, temp);
					g_free (temp);

					if (tab_affichage_ope[i][column_number] == active_sort)
						gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);

					g_object_set_data (G_OBJECT (menu_item),
									   "no_sort",
									   GINT_TO_POINTER (tab_affichage_ope[i][column_number]));
					g_signal_connect (G_OBJECT(menu_item),
									  "activate",
									  G_CALLBACK (gsb_transactions_list_change_sort_type),
									  no_column);

					gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
					gtk_widget_show (menu_item);
				}
			}
		}

		if (menu)
		{
			gfloat alignment;

			menu_item = gtk_separator_menu_item_new ();
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
			gtk_widget_show (menu_item);

			/* alignement */
			alignment = gtk_tree_view_column_get_alignment (gtk_tree_view_get_column
															(GTK_TREE_VIEW (transactions_tree_view),
															 column_number));
			menu_item = gtk_menu_item_new_with_label (_("alignment: "));
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
			gtk_widget_show (menu_item);

			menu_item = gtk_separator_menu_item_new ();
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
			gtk_widget_show (menu_item);

			menu_item = gtk_radio_menu_item_new_with_label (NULL, _("LEFT"));
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
			if (alignment == COLUMN_LEFT)
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
			g_object_set_data (G_OBJECT (menu_item),
							   "alignment",
							   GINT_TO_POINTER (GTK_JUSTIFY_LEFT));
			g_signal_connect (G_OBJECT(menu_item),
							  "activate",
							  G_CALLBACK (gsb_transactions_list_change_alignment),
							  no_column);
			gtk_widget_show (menu_item);

			menu_item = gtk_radio_menu_item_new_with_label_from_widget (GTK_RADIO_MENU_ITEM
																		(menu_item),
																		_("CENTER"));
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
			if (alignment == COLUMN_CENTER)
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
			g_object_set_data (G_OBJECT (menu_item),
							   "alignment",
							   GINT_TO_POINTER (GTK_JUSTIFY_CENTER));
			g_signal_connect (G_OBJECT(menu_item),
							  "activate",
							  G_CALLBACK (gsb_transactions_list_change_alignment),
							  no_column);
			gtk_widget_show (menu_item);

			menu_item = gtk_radio_menu_item_new_with_label_from_widget (GTK_RADIO_MENU_ITEM
																		(menu_item),
																		_("RIGHT"));
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
			if (alignment == COLUMN_RIGHT)
				gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
			g_object_set_data (G_OBJECT (menu_item),
							   "alignment",
							   GINT_TO_POINTER (GTK_JUSTIFY_RIGHT));
			g_signal_connect (G_OBJECT(menu_item),
							  "activate",
							  G_CALLBACK (gsb_transactions_list_change_alignment),
							  no_column);
			gtk_widget_show (menu_item);
			gtk_widget_show (menu);

			gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);
		}
    }

	return FALSE;
}

/**
 * called when the size of the tree view changed, to keep the same ration
 * between the columns
 *
 * \param tree_view	the tree view of the transactions list
 * \param allocation	the new size
 * \param null
 *
 * \return FALSE
 **/
static void gsb_transactions_list_size_allocate (GtkWidget *tree_view,
												 GtkAllocation *allocation,
												 GrisbiWinRun *w_run)
{
    gint i;

	if (gsb_gui_navigation_get_current_page () != GSB_ACCOUNT_PAGE)
	{
		return;
	}

    if (allocation->width == current_tree_view_width)
    {
		gint somme = 0;
		gint tmp_width;

		/* size of the tree view didn't change, but we received an allocated signal
         * it happens several times, and especially when we change the columns,
         * so we update the colums */

        /* sometimes, when the list is not visible, he will set all the columns to 1%... we block that here */
        if (gtk_tree_view_column_get_width (transactions_tree_view_columns[0]) == 1)
            return;

        for (i = 0 ; i<CUSTOM_MODEL_VISIBLE_COLUMNS -1; i++)
		{
			tmp_width = (gtk_tree_view_column_get_width
						 (transactions_tree_view_columns[i]) * 100) / allocation->width + 1;

			if (tmp_width != transaction_col_width[i])
			{
				transaction_col_width[i] = tmp_width;
				gsb_file_set_modified (TRUE);
			}
			somme+= transaction_col_width[i];
		}

		transaction_col_width[i] = 100 - somme;


        return;
    }

    /* the size of the tree view changed, we keep the ration between the columns,
     * we don't set the size of the last column to avoid the calculate problems,
     * it will take the end of the width alone */
    current_tree_view_width = allocation->width;

	for (i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS -1 ; i++)
    {
        gint width;

        width = (transaction_col_width[i] * (allocation->width))/ 100;
        if (width > 0)
            gtk_tree_view_column_set_fixed_width (transactions_tree_view_columns[i], width);
    }

	grisbi_win_form_label_align_right (allocation);
	//~ gsb_transactions_list_update_tree_view (gsb_form_get_account_number (), TRUE);
}

/**
 * gère le clavier sur la liste des opés
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean gsb_transactions_list_key_press (GtkWidget *widget,
												 GdkEventKey *ev)
{
    gint account_number;
    gint transaction_number;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
    account_number = gsb_gui_navigation_get_current_account ();

	switch (ev->keyval)
    {
		case GDK_KEY_Return :   /* entrée */
		case GDK_KEY_KP_Enter :
		case GDK_KEY_Tab :

			gsb_transactions_list_edit_transaction (gsb_data_account_get_current_transaction_number
													(account_number));
			break;

		case GDK_KEY_Up :       /* touches flèche haut */
		case GDK_KEY_KP_Up :

			transaction_list_select_up (FALSE);
			break;

		case GDK_KEY_Down :     /* touches flèche bas */
		case GDK_KEY_KP_Down :

			transaction_list_select_down (FALSE);
			break;

		case GDK_KEY_Delete:    /*  del  */
			gsb_transactions_list_delete_transaction (gsb_data_account_get_current_transaction_number
													  (account_number),
							TRUE);
			break;

		case GDK_KEY_F:         /* touche F*/
		case GDK_KEY_f:         /* touche f */
			if ((ev->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
			{
				transaction_number = gsb_data_account_get_current_transaction_number (account_number);
				gsb_transactions_list_search (NULL, GINT_TO_POINTER (transaction_number));
			}
			break;

		case GDK_KEY_P:         /* touche P */
		case GDK_KEY_p:         /* touche p */
		case GDK_KEY_F12:       /* touche F12 pour pointer dépointer comme avec <ctrl>p*/

			if ((ev->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK
				||
				ev->keyval == GDK_KEY_F12)
			{
				gsb_transactions_list_switch_mark (gsb_data_account_get_current_transaction_number
												   (account_number));
				transaction_list_set_balances ();
			}
			break;

		case GDK_KEY_r:         /* touche r */
		case GDK_KEY_R:         /* touche R */

			if ((ev->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
			{
				gsb_transactions_list_switch_R_mark (gsb_data_account_get_current_transaction_number
													 (account_number));
				transaction_list_set_balances ();
			}
			break;

		case GDK_KEY_t:         /* touche t */
		case GDK_KEY_T:         /* touche T */
			transaction_list_select (-1);
			break;

		case GDK_KEY_space:
			transaction_number = gsb_data_account_get_current_transaction_number (account_number);
			if (transaction_number > 0)
			{
				if (w_run->equilibrage)
				{
					/* we are reconciling, so mark/unmark the transaction */
					gsb_transactions_list_switch_mark (transaction_number);
					transaction_list_select_down (FALSE);
				}
				else
				{
					/* space open/close a split */
					gsb_transactions_list_switch_expander (transaction_number);
				}
			}
			break;

		case GDK_KEY_Left:
			/* if we press left, give back the focus to the tree at left */
			gtk_widget_grab_focus (gsb_gui_navigation_get_tree_view ());
			break;

		case GDK_KEY_Home:
		case GDK_KEY_KP_Home:
			gtk_tree_view_scroll_to_point (GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()), 0, 0);
			break;

		case GDK_KEY_End:
		case GDK_KEY_KP_End:
			gtk_tree_view_scroll_to_point (GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()), -1, 1024);
			break;
    }

    return TRUE;
}

/**
 * create the tree view from the CustomList
 *
 * \param model	the CustomList
 *
 * \return the tree_view
 **/
static GtkWidget *gsb_transactions_list_create_tree_view (GtkTreeModel *model)
{
    GtkWidget *tree_view;
    gint i;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

    tree_view = gtk_tree_view_new ();

    /*  we cannot do a selection */
    gtk_tree_selection_set_mode (GTK_TREE_SELECTION (gtk_tree_view_get_selection
													 (GTK_TREE_VIEW (tree_view))),
								 GTK_SELECTION_NONE);

    gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (tree_view), TRUE);

    /* check the buttons on the list */
    g_signal_connect (G_OBJECT (tree_view),
		              "button_press_event",
		              G_CALLBACK (gsb_transactions_list_button_press),
		              NULL);

    /* check the keys on the list */
    g_signal_connect (G_OBJECT (tree_view),
		              "key-press-event",
		              G_CALLBACK (gsb_transactions_list_key_press),
		              NULL);

    g_signal_connect (G_OBJECT (tree_view),
		              "size-allocate",
		              G_CALLBACK (gsb_transactions_list_size_allocate),
		              w_run);

    /* we create the columns of the tree view */
    gsb_transactions_list_create_tree_view_columns ();

    for (i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++)
    {
        GtkWidget *button;
        GtkTreeViewColumn *col;

        col = GTK_TREE_VIEW_COLUMN (transactions_tree_view_columns[i]);
        gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), col);

        gtk_tree_view_column_set_clickable (col, TRUE);

        /* set the tooltips */
        button = gtk_tree_view_column_get_button (col);
        gtk_widget_set_tooltip_text (button, tips_col_liste_operations[i]);

        g_signal_connect (G_OBJECT (button),
                          "button-press-event",
                          G_CALLBACK (gsb_transactions_list_title_column_button_press),
                          GINT_TO_POINTER (i));

        /* use the click to sort the list */
        g_signal_connect (G_OBJECT (col),
                          "clicked",
                          G_CALLBACK (gsb_transactions_list_change_sort_column),
                          GINT_TO_POINTER (i));
    }

    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));

    /* return */
    return tree_view;
}

/**
 * Delete the archived transactions in the tree view
 *
 * \param account_number
 * \param archive_number
 *
 * \return FALSE
 * **/
static gboolean gsb_transactions_list_delete_archived_transactions (gint account_number,
																	gint archive_number)
{
	GSList *tmp_list;

	tmp_list = gsb_data_transaction_get_transactions_list ();
	while (tmp_list)
	{
	    gint transaction_number;

        transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);

	    tmp_list = tmp_list->next;

	    if (gsb_data_transaction_get_account_number (transaction_number) == account_number
			&&
			gsb_data_transaction_get_archive_number (transaction_number) == archive_number)
        {
            gsb_data_transaction_remove_transaction_in_transaction_list (transaction_number);
            transaction_list_remove_transaction (transaction_number);
        }
	}

    return FALSE;
}

/**
 * delete an import_rule
 *
 * \param import_rule_number the import_rule we want to delete
 *
 * \return TRUE if ok
 **/
static void gsb_transactions_list_delete_import_rule (gint import_rule_number)
{
	gchar *tmp_str;

	tmp_str = g_strdup (_("Do you really want to delete this file import rule?"));

    if (!dialogue_conditional_yes_no_with_items ("tab_delete_msg", "delete-rule", tmp_str))
    {
        g_free (tmp_str);

		return;
    }
    g_free(tmp_str);

    if (gsb_data_import_rule_remove (import_rule_number))
	{
		GSList	*tmp_list;
		gint number;

		/* on met à jour la barre de menu */
		tmp_list = gsb_data_import_rule_get_from_account (gsb_gui_navigation_get_current_account ());
		number = g_slist_length (tmp_list);
		if (!number)
			gtk_widget_hide (menu_import_rules);

		gsb_file_set_modified (TRUE);
	}
}

/**
 * remove the orphan transactions
 *
 * \param orphan list
 *
 * \return void
 **/
static void gsb_transactions_list_process_orphan_list (GSList *orphan_list)
{
    GSList *tmp_list;
    gchar *string = NULL;
    GArray *garray;

    garray = g_array_new (FALSE, FALSE, sizeof (gint));
    tmp_list = orphan_list;
    while (tmp_list)
    {
        gint transaction_number;

        transaction_number = GPOINTER_TO_INT (tmp_list->data);

        /* on sauvegarde le numéro de l'opération */
        g_array_append_val (garray, transaction_number);
        if (string == NULL)
            string = utils_str_itoa (transaction_number);
        else
		{
			gchar *tmp_str1, *tmp_str2;
			tmp_str1 = utils_str_itoa (transaction_number);
			tmp_str2 = g_strconcat (string, " - ", tmp_str1, NULL);
			g_free(tmp_str1);
			g_free(string);
			string = tmp_str2;
		}

        tmp_list = tmp_list->next;
    }

    /* if string is not null, there is still some children
     * which didn't find their mother. show them now */
    if (string)
    {
        gchar *message;
        gint result;

        message = g_strdup_printf (_("Some children didn't find their mother in the list, "
                        "this shouldn't happen and there is probably a bug behind that.\n\n"
                        "The concerned children number are:\n %s\n\n"
                        "Do you want to delete it?"),
                        string);

        result = dialogue_yes_no (message, _("Remove orphan children"), GTK_RESPONSE_CANCEL);

        if (result == TRUE)
        {
            guint i;

            for (i = 0; i < garray->len; i++)
                gsb_data_transaction_remove_transaction (g_array_index (garray, gint, i));
        }

        g_free (message);
        g_free (string);
        g_array_free (garray, TRUE);
    }
}

/**
 * fill the new store with the all the transactions
 * normally called at the opening of a file
 *
 * \param
 *
 * \return FALSE
 **/
static gboolean gsb_transactions_list_fill_model (void)
{
    GSList *tmp_list;
    gint transaction_number;

    devel_debug (NULL);

    /* add the transations which represent the archives to the store
     * 1 line per archive and per account */
    gsb_transactions_list_fill_archive_store ();

    orphan_child_transactions = NULL;

    /* add the transactions to the store */
    tmp_list = gsb_data_transaction_get_transactions_list ();
    while (tmp_list)
    {
	transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
	transaction_list_append_transaction (transaction_number);
	tmp_list = tmp_list->next;
    }

    /* if orphan_child_transactions if filled, there are some children which didn't fing their
     * mother, we try again now that all the mothers are in the model */
    if (orphan_child_transactions)
    {
        gsb_transactions_list_process_orphan_list (orphan_child_transactions);

        g_slist_free (orphan_child_transactions);
        orphan_child_transactions = NULL;
    }
    return FALSE;
}

/**
 * Cette fonction supprime les transactions concernées et les remplace par une ligne d'archive
 *
 *
 * \param
 * \param
 *
 * \return always FALSE
 **/
static gboolean gsb_transactions_list_hide_transactions_in_archive_line (GtkWidget *button,
																		 gpointer null)
{
    GSList *tmp_list;
    gint account_number;

    account_number = gsb_gui_navigation_get_current_account ();

    tmp_list = gsb_data_archive_store_get_archives_list ();

    while (tmp_list)
    {
        StoreArchive *archive_store;

        archive_store = tmp_list->data;

        if (archive_store->account_number == account_number
         &&
         archive_store->transactions_visibles == TRUE)
        {
            transaction_list_append_archive (archive_store->archive_store_number);
            gsb_transactions_list_delete_archived_transactions (account_number,
																archive_store->archive_number);
            gsb_data_archive_store_set_transactions_visibles (archive_store->archive_number,
															  account_number, FALSE);
        }

        tmp_list = tmp_list->next;
    }

    gsb_transactions_list_set_visible_archived_button (FALSE);

    gsb_transactions_list_update_tree_view (account_number, TRUE);

    return FALSE;
}

/**
 * save the transactions tree_view
 *
 * \param tree_view
 *
 * \return
 **/
static void gsb_transactions_list_set_tree_view (GtkWidget *tree_view)
{
    transactions_tree_view = tree_view;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void gsb_transactions_list_edit_import_rule (gint import_rule_number)
{
	csv_template_rule_edit (GTK_WINDOW (grisbi_app_get_active_window (NULL)), import_rule_number);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static gboolean popup_transaction_rules_menu (GtkWidget *button,
											  gpointer null)
{
    GtkWidget *menu, *menu_item;
    GSList *tmp_list;
    gint current_account;
    gint i = 0;

    current_account = gsb_gui_navigation_get_current_account ();
    menu = gtk_menu_new ();

    tmp_list = gsb_data_import_rule_get_from_account (current_account);

    while (tmp_list)
    {
        gint rule;
		const gchar *type;

        rule = gsb_data_import_rule_get_number (tmp_list->data);
		type = gsb_data_import_rule_get_type (rule);

        if (i > 0)
        {
            menu_item = gtk_separator_menu_item_new ();
            gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
        }

        menu_item = gtk_menu_item_new_with_label (gsb_data_import_rule_get_name (rule));
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
        g_signal_connect_swapped (G_OBJECT (menu_item),
								  "activate",
								  G_CALLBACK (gsb_import_by_rule),
								  GINT_TO_POINTER (rule));

		if (g_strcmp0 (type, "CSV") == 0)
		{
			menu_item = gtk_menu_item_new_with_label (_("Edit the rule"));
			g_signal_connect_swapped (G_OBJECT (menu_item),
									  "activate",
									  G_CALLBACK (gsb_transactions_list_edit_import_rule),
									  GINT_TO_POINTER (rule));
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
		}

        menu_item = gtk_menu_item_new_with_label (_("Remove the rule"));
        g_signal_connect_swapped (G_OBJECT (menu_item),
								  "activate",
								  G_CALLBACK (gsb_transactions_list_delete_import_rule),
								  GINT_TO_POINTER (rule));

        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

        i++;

        tmp_list = tmp_list->next;
    }

    gtk_widget_show_all (menu);

	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);

    return FALSE;
}

/**
 * Fonction appelée lorsqu'on change le nombre de lignes des opérations
 *
 * \param   menu_item concerné
 * \param   pointeur sur l'index du bouton
 *
 * \return
 **/
static void gsb_transaction_list_radio_button_activate (GtkMenuItem *menu_item,
														gpointer data)
{
    if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (menu_item)))
    {
        gint demande = 0;

        if (data)
            demande = GPOINTER_TO_INT (data);
        gsb_transactions_list_change_aspect_liste (demande);
    }
}

/**
 * Création du bouton de sélection de l'affichage de la liste des opérations.
 *
 * \param   bouton sur lequel on attache le menu
 * \param   pointeur NULL
 *
 * \return  FALSE
 **/
static gboolean popup_transaction_view_mode_menu (GtkWidget *button,
												  gpointer null)
{
    GSList *group = NULL;
    GtkWidget *menu;
    GtkWidget *menu_item, *menu_item_1, *menu_item_2, *menu_item_3;
    gint current_account;
    gint nb_rows_by_transaction;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
    current_account = gsb_gui_navigation_get_current_account ();
    nb_rows_by_transaction = gsb_data_account_get_nb_rows (current_account);

    menu = gtk_menu_new ();

    menu_item_1 = gtk_radio_menu_item_new_with_label (group, _("Simple view"));
    group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menu_item_1));
    if (nb_rows_by_transaction == 1)
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item_1), TRUE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item_1);

    menu_item_2 = gtk_radio_menu_item_new_with_label (group, _("Two lines view"));
    group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menu_item_2));
    if (nb_rows_by_transaction == 2)
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item_2), TRUE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item_2);

    menu_item_3 = gtk_radio_menu_item_new_with_label (group, _("Three lines view"));
    group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menu_item_3));
    if (nb_rows_by_transaction == 3)
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item_3), TRUE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item_3);

    menu_item = gtk_radio_menu_item_new_with_label (group, _("Complete view"));
    if (nb_rows_by_transaction == 4)
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

    g_signal_connect (G_OBJECT (menu_item),
                        "activate",
                        G_CALLBACK (gsb_transaction_list_radio_button_activate),
                        GINT_TO_POINTER (4));

    g_signal_connect (G_OBJECT (menu_item_1),
                        "activate",
                        G_CALLBACK (gsb_transaction_list_radio_button_activate),
                        GINT_TO_POINTER (1));

    g_signal_connect (G_OBJECT (menu_item_2),
                        "activate",
                        G_CALLBACK (gsb_transaction_list_radio_button_activate),
                        GINT_TO_POINTER (2));

    g_signal_connect (G_OBJECT (menu_item_3),
                        "activate",
                        G_CALLBACK (gsb_transaction_list_radio_button_activate),
                        GINT_TO_POINTER (3));

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), gtk_separator_menu_item_new ());

    menu_item = gtk_check_menu_item_new_with_label (_("Show reconciled transactions"));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item),
				        gsb_data_account_get_r (current_account));
    if (w_run->equilibrage)
        gtk_widget_set_sensitive (menu_item, FALSE);
    else
        gtk_widget_set_sensitive (menu_item, TRUE);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
    g_signal_connect (G_OBJECT (menu_item),
                        "activate",
			            G_CALLBACK (gsb_menu_gui_toggle_show_reconciled),
                        NULL);

    menu_item = gtk_check_menu_item_new_with_label (_("Show lines archives"));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item),
				        gsb_data_account_get_l (current_account));
    if (w_run->equilibrage)
        gtk_widget_set_sensitive (menu_item, FALSE);
    else
        gtk_widget_set_sensitive (menu_item, TRUE);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
    g_signal_connect (G_OBJECT (menu_item),
                        "activate",
			            G_CALLBACK (gsb_menu_gui_toggle_show_archived),
                        NULL);

    gtk_menu_set_active (GTK_MENU(menu),
			  gsb_data_account_get_nb_rows (gsb_gui_navigation_get_current_account ()));

    gtk_widget_show_all (menu);

	gtk_menu_popup_at_pointer (GTK_MENU (menu), NULL);

    /* return */
    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static GtkWidget *gsb_transactions_list_new_toolbar (void)
{
    GtkWidget *toolbar;
    GtkToolItem *item;
    GtkToolItem *separator;
    gint account_number;

    toolbar = gtk_toolbar_new ();

    /* new transaction button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-new-transaction-24.png", _("New transaction"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Blank the form to create a new transaction"));
    g_signal_connect (G_OBJECT (item),
                        "clicked",
                        G_CALLBACK (gsb_transactions_list_select_new_transaction),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* delete button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-delete-24.png", _("Delete"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Delete selected transaction"));
    g_signal_connect (G_OBJECT (item),
                        "clicked",
                        G_CALLBACK (gsb_transactions_list_remove_transaction),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* edit button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-edit-24.png", _("Edit"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Edit current transaction"));
    g_signal_connect (G_OBJECT (item),
                        "clicked",
                        G_CALLBACK (gsb_transactions_list_edit_current_transaction),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* reconcile button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-reconciliation-24.png", _("Reconcile"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Start account reconciliation"));
    g_signal_connect (G_OBJECT (item),
					  "clicked",
					  G_CALLBACK (widget_reconcile_run_reconciliation),
					  NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* print button */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-print-24.png", _("Print"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Print the transactions list"));
    g_signal_connect (G_OBJECT (item),
                        "clicked",
                        G_CALLBACK (print_transactions_list),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* select the number of lines */
    item = utils_buttons_tool_button_new_from_image_label ("gtk-select-color-24.png", _("View"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item), _("Change display mode of the list"));
    g_signal_connect (G_OBJECT (item),
                        "clicked",
                        G_CALLBACK (popup_transaction_view_mode_menu),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);

    /* select the rule */
    menu_import_rules = GTK_WIDGET (utils_buttons_tool_button_new_from_image_label ("gtk-execute-24.png", _("Import rules")));
    gtk_widget_set_tooltip_text (GTK_WIDGET (menu_import_rules), _("Quick file import by rules"));
    g_signal_connect (G_OBJECT (menu_import_rules),
                        "clicked",
                        G_CALLBACK (popup_transaction_rules_menu),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), GTK_TOOL_ITEM (menu_import_rules), -1);

    /* add a separator */
    separator = gtk_separator_tool_item_new ();
    gtk_separator_tool_item_set_draw (GTK_SEPARATOR_TOOL_ITEM (separator), FALSE);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (separator), TRUE);

    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), separator, -1);

    /* archive button */
    item = utils_buttons_tool_button_new_from_image_label ("gsb-archive-24.png", _("Recreates archive"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (item),
                        _("Recreates the line of the archive and hiding the transactions"));
    g_signal_connect (G_OBJECT (item),
                        "clicked",
                        G_CALLBACK (gsb_transactions_list_hide_transactions_in_archive_line),
                        NULL);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
    g_object_set_data (G_OBJECT (toolbar), "archived_button", item);

    account_number = gsb_gui_navigation_get_current_account ();
	gsb_transactions_list_show_menu_import_rule (account_number);

    if (gsb_data_archive_store_account_have_transactions_visibles (account_number))
        gtk_widget_set_visible (GTK_WIDGET (item), TRUE);
    else
        gtk_widget_set_visible (GTK_WIDGET (item), FALSE);

    return (toolbar);
}

/**
 *  Convert transaction to a template of scheduled transaction.
 *
 * \param transaction Transaction to use as a template.
 *
 * \return the number of the scheduled transaction
 **/
static gint gsb_transactions_list_convert_to_scheduled (gint transaction_number)
{
    gint scheduled_number;

    scheduled_number = gsb_data_scheduled_new_scheduled ();

	if (!scheduled_number)
		return FALSE;

    gsb_data_scheduled_set_account_number (scheduled_number,
										   gsb_data_transaction_get_account_number (transaction_number));
    gsb_data_scheduled_set_date (scheduled_number,
								 gsb_data_transaction_get_date (transaction_number));
    gsb_data_scheduled_set_amount (scheduled_number,
								   gsb_data_transaction_get_amount (transaction_number));
    gsb_data_scheduled_set_currency_number (scheduled_number,
											gsb_data_transaction_get_currency_number (transaction_number));
    gsb_data_scheduled_set_payee_number (scheduled_number,
										 gsb_data_transaction_get_payee_number (transaction_number));
    gsb_data_scheduled_set_category_number (scheduled_number,
											gsb_data_transaction_get_category_number (transaction_number));
    gsb_data_scheduled_set_sub_category_number (scheduled_number,
												gsb_data_transaction_get_sub_category_number (transaction_number));

    /* pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
    /* mais si categ et sous categ sont à 0 et que ce n'est pas un virement ni une ventil, compte_virement = -1 */
    /* on va changer ça la prochaine version, dès que c'est pas un virement->-1 */

    if (gsb_data_transaction_get_contra_transaction_number (transaction_number) > 0)
    {
		/* 	c'est un virement, on met la relation et on recherche le type de la contre opération */
		gint contra_transaction_number;

		gsb_data_scheduled_set_account_number_transfer (scheduled_number,
														gsb_data_transaction_get_contra_transaction_account
														(transaction_number));

		contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number);

		gsb_data_scheduled_set_contra_method_of_payment_number (scheduled_number,
																gsb_data_transaction_get_method_of_payment_number
																(contra_transaction_number));
    }
    else if (!gsb_data_scheduled_get_category_number (scheduled_number)
			 &&
			 !gsb_data_transaction_get_split_of_transaction (transaction_number))
		gsb_data_scheduled_set_account_number_transfer (scheduled_number, -1);

    gsb_data_scheduled_set_notes (scheduled_number,
								  gsb_data_transaction_get_notes (transaction_number));
    gsb_data_scheduled_set_method_of_payment_number (scheduled_number,
													 gsb_data_transaction_get_method_of_payment_number
													 (transaction_number));
    gsb_data_scheduled_set_method_of_payment_content (scheduled_number,
													  gsb_data_transaction_get_method_of_payment_content
													  (transaction_number));

    gsb_data_scheduled_set_financial_year_number (scheduled_number,
												  gsb_data_transaction_get_financial_year_number
												  (transaction_number));
    gsb_data_scheduled_set_budgetary_number (scheduled_number,
											 gsb_data_transaction_get_budgetary_number (transaction_number));
    gsb_data_scheduled_set_sub_budgetary_number (scheduled_number,
												 gsb_data_transaction_get_sub_budgetary_number (transaction_number));
    gsb_data_scheduled_set_split_of_scheduled (scheduled_number,
											   gsb_data_transaction_get_split_of_transaction (transaction_number));

    /* par défaut, on met en manuel, pour éviter si l'utilisateur se gourre dans la date, */
    /* (c'est le cas, à 0 avec g_malloc0) */
    /* que l'opé soit enregistrée immédiatement ; de même on le met en mensuel par défaut */
    /* pour la même raison */

    gsb_data_scheduled_set_frequency (scheduled_number, 2);

    /* on récupère les opés de ventil si c'était une opé ventilée */
    if (gsb_data_scheduled_get_split_of_scheduled (scheduled_number))
		gsb_transactions_list_splitted_to_scheduled (transaction_number, scheduled_number);

	return scheduled_number;
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * get the transactions tree_view
 *
 * \param
 *
 * \return the GtkTreeView
 **/
GtkWidget *gsb_transactions_list_get_tree_view (void)
{
    return (transactions_tree_view);
}

/**
 * update the tree view, ie :
 * 	filter it according to the account
 * 	sort it
 * 	colorize it
 * 	set the balances
 * 	keep the selected transaction if we want
 * this should be called all the time, except when we need only 1 of all of this
 *
 * \param account_number
 * \param keep_selected_transaction	TRUE if we want to keep the selected transaction
 * 					else we need to select another transaction after that call
 *
 * \return
 **/
void gsb_transactions_list_update_tree_view (gint account_number,
											 gboolean keep_selected_transaction)
{
    gint selected_transaction = 0;
	GrisbiAppConf *a_conf;

	/* called sometimes with gsb_gui_navigation_get_current_account, so check we are
     * on an account */
	if (account_number <= 0)
		return;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	if (keep_selected_transaction)
		selected_transaction = transaction_list_select_get ();

	/* Fix bug 2172 */
	//~ if (transaction_list_filter (account_number))
	//~ {
		transaction_list_filter (account_number);
	//~ }
    transaction_list_set_balances ();
    transaction_list_sort ();
    transaction_list_colorize ();
    if (a_conf->show_transaction_gives_balance)
		transaction_list_set_color_jour (account_number);
	if (keep_selected_transaction)
	{
        transaction_list_select (selected_transaction);
	}
    else
	{
        transaction_list_select (-1);
	}
}

/**
 * Create the transaction window with all components needed.
 *
 * \param
 *
 * \return
 **/
GtkWidget *gsb_transactions_list_creation_fenetre_operations (void)
{

    GtkWidget *win_operations;
    GtkWidget *frame;
	GtkWidget *vbox_transactions_list = NULL; /* adr de la vbox qui contient les opés de chaque compte */

    /* la fenetre des opé est une vbox : la barre d'outils en haut et la liste en bas */
    win_operations = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);

    /* frame pour la barre d'outils */
    frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (win_operations), frame, FALSE, FALSE, 0);

    /* création de la barre d'outils */
    transaction_toolbar = gsb_transactions_list_new_toolbar ();
    gtk_container_add (GTK_CONTAINER (frame), transaction_toolbar);

    /* vbox_transactions_list will contain the tree_view, we will see later to set it directly */
    vbox_transactions_list = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	grisbi_win_set_vbox_transactions_list (NULL, vbox_transactions_list);

    gtk_box_pack_start (GTK_BOX (win_operations), vbox_transactions_list, TRUE, TRUE, 0);

    gtk_widget_show_all (win_operations);

    return (win_operations);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_gui_transaction_toolbar_set_style (gint toolbar_style)
{
    gtk_toolbar_set_style (GTK_TOOLBAR (transaction_toolbar), toolbar_style);
}

/**
 * create fully the gui list and fill it
 *
 * \param
 *
 * \return the widget which contains the list, to set at the right place
 **/
GtkWidget *gsb_transactions_list_make_gui_list (void)
{
    GtkWidget *tree_view;
    GtkWidget *scrolled_window;

    /* we have first to create and fill the model */
    gsb_transactions_list_fill_model ();

    /* we add the tree view in a scrolled window which will be returned */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);

    /* and now we can create the tree_view */
    tree_view = gsb_transactions_list_create_tree_view (GTK_TREE_MODEL (transaction_model_get_model ()));
	gtk_widget_set_margin_end (tree_view, MARGIN_END);
    gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);

    /* we save the values */
    gsb_transactions_list_set_tree_view (tree_view);

    gtk_widget_show_all (scrolled_window);

	return scrolled_window;
}

/**
 * update the titles of the tree view
 *
 * \param
 *
 * \return
 **/
void gsb_transactions_list_update_titres_tree_view (void)
{
    gint i;

    for (i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++)
    {
        GtkTreeViewColumn *col;
        GtkWidget *button;

        col = GTK_TREE_VIEW_COLUMN (transactions_tree_view_columns[i]);
        gtk_tree_view_column_set_title (col, (titres_colonnes_liste_operations[i]));

        button = gtk_tree_view_column_get_button  (col);
        gtk_widget_set_tooltip_text (button, tips_col_liste_operations[i]);
    }
}

/**
 * fill the store with the archive
 * it's only 1 line per archive, with a name,
 * a date (the initial date) and an amount (the balance of the archive for the account)
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_transactions_list_fill_archive_store (void)
{
    GSList *tmp_list;

    devel_debug (NULL);

    tmp_list = gsb_data_archive_store_get_archives_list ();
    while (tmp_list)
    {
		gint archive_store_number;

		/* get the store archive struct */
		archive_store_number = gsb_data_archive_store_get_number (tmp_list->data);
		transaction_list_append_archive (archive_store_number);
		tmp_list = tmp_list->next;
    }

	return FALSE;
}

/**
 * append a new transaction in the tree_view
 * use gsb_transactions_list_append_transaction and do all the stuff
 * 	arround that, ie calculate the balances, show or not the line...
 * 	so this function is very good to append 1 transactions, but to append
 * 	several transactions, it's better to use gsb_transactions_list_append_transaction
 * 	and do the stuff separatly
 *
 * if the transaction is a split, append a white line and open
 * the split to see the daughters
 *
 * \param transaction_number
 * \param update_tree_view	should be TRUE, except if we append a lot of transactions (import...)
 * 				this must be FALSE to avoid time consuming, and need to update tree view
 * 				later
 *
 * \return FALSE
 **/
gboolean gsb_transactions_list_append_new_transaction (gint transaction_number,
													   gboolean update_tree_view)
{
    gint account_number;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
    account_number = gsb_data_transaction_get_account_number (transaction_number);

    /* append the transaction to the tree view */
    transaction_list_append_transaction (transaction_number);

    /* update the transaction list only if the account is showed,
     * else it's because we execute a scheduled transaction and all
     * of that stuff will be done when we will show the account */
    if (update_tree_view
		&&
		gsb_gui_navigation_get_current_account () == account_number
		&&
		!gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
        gsb_transactions_list_update_tree_view (account_number, TRUE);
        gsb_data_account_colorize_current_balance (account_number);

        /* if it's a mother, open the expander */
        if (gsb_data_transaction_get_split_of_transaction (transaction_number))
            gsb_transactions_list_switch_expander (transaction_number);
    }

    /* on réaffichera l'accueil */
    w_run->mise_a_jour_liste_comptes_accueil = TRUE;
    w_run->mise_a_jour_soldes_minimaux = TRUE;
    w_run->mise_a_jour_fin_comptes_passifs = TRUE;

	return FALSE;
}

/**
 * take in a transaction the content to set in a cell of the transaction's list
 * all the value are dupplicate and have to be freed after use (except when NULL)
 *
 * \param transaction_number
 * \param cell_content_number what we need in the transaction
 *
 * \return a newly allocated string which represent the content of the transaction, or NULL
 **/
gchar *gsb_transactions_list_grep_cell_content (gint transaction_number,
												gint cell_content_number)
{
    gint account_currency;

    /* for a child of split, we show only the category (instead of party or category), the
     * debit and credit, nothing else */
    if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
    {
		switch (cell_content_number)
		{
			case ELEMENT_PARTY:
			/* here want to show the category instead of the payee */
			cell_content_number = ELEMENT_CATEGORY;
			break;

			case ELEMENT_CATEGORY:
			case ELEMENT_DEBIT:
			case ELEMENT_CREDIT:
			case ELEMENT_EXERCICE:		/* See Bug 1992 */
			break;

			default:
			return NULL;
		}
    }

    switch (cell_content_number)
    {
		/* mise en forme de la date */
		case ELEMENT_DATE:
			return gsb_format_gdate (gsb_data_transaction_get_date (transaction_number));

		/* mise en forme de la date de valeur */
		case ELEMENT_VALUE_DATE:
			return gsb_format_gdate (gsb_data_transaction_get_value_date (transaction_number));

		/* mise en forme du tiers */
		case ELEMENT_PARTY:
			return (my_strdup (gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
														(transaction_number),
														TRUE)));

		/* mise en forme de l'ib */
		case ELEMENT_BUDGET:
			return (gsb_data_budget_get_name (gsb_data_transaction_get_budgetary_number
														 (transaction_number),
														 gsb_data_transaction_get_sub_budgetary_number
														 (transaction_number),
														 NULL));

		case ELEMENT_DEBIT:
		case ELEMENT_CREDIT:
			/* give the amount of the transaction on the transaction currency */
			if ((cell_content_number == ELEMENT_DEBIT
				 &&
				 gsb_data_transaction_get_amount (transaction_number).mantissa < 0)
				||
				(cell_content_number == ELEMENT_CREDIT
				 &&
				 gsb_data_transaction_get_amount (transaction_number).mantissa >= 0))
				return utils_real_get_string_with_currency (gsb_real_abs (gsb_data_transaction_get_amount
																		  (transaction_number)),
															gsb_data_transaction_get_currency_number
															(transaction_number),
															TRUE);
			else
			return NULL;
			break;

		case ELEMENT_BALANCE:
			return NULL;

		case ELEMENT_AMOUNT:
			/* give the amount of the transaction in the currency of the account */
			account_currency = gsb_data_account_get_currency (gsb_data_transaction_get_account_number
															  (transaction_number));
			if (account_currency != gsb_data_transaction_get_currency_number (transaction_number))
			{
				gchar* tmp_str;
				gchar* result;

				tmp_str = utils_real_get_string (gsb_data_transaction_get_adjusted_amount_for_currency
												(
												 transaction_number,
												 account_currency,
												 gsb_data_currency_get_floating_point (account_currency)));
				result = g_strconcat ("(",
									  tmp_str,
									  " ",
									  gsb_data_currency_get_nickname_or_code_iso (account_currency),
									  ")",
										NULL);
				g_free (tmp_str);

				return result;
			}
			else
				return NULL;
			break;

		/* mise en forme du moyen de paiement */
		case ELEMENT_PAYMENT_TYPE:
			return (my_strdup (gsb_data_payment_get_name (gsb_data_transaction_get_method_of_payment_number
														  (transaction_number))));

		/* mise en forme du no de rapprochement */
		case ELEMENT_RECONCILE_NB:
			return (my_strdup (gsb_data_reconcile_get_name (gsb_data_transaction_get_reconcile_number
															(transaction_number))));

		/* mise en place de l'exo */
		case ELEMENT_EXERCICE:
			return (my_strdup (gsb_data_fyear_get_name (gsb_data_transaction_get_financial_year_number
														(transaction_number))));

		/* mise en place des catégories */
		case ELEMENT_CATEGORY:

			return (gsb_data_transaction_get_category_real_name (transaction_number));

		/* mise en forme R/P */
		case ELEMENT_MARK:
			if (gsb_data_transaction_get_marked_transaction (transaction_number)== OPERATION_POINTEE)
				return (my_strdup (_("P")));
			else
			{
				if (gsb_data_transaction_get_marked_transaction (transaction_number)== OPERATION_TELEPOINTEE)
					return (my_strdup (_("T")));
				else
				{
					if (gsb_data_transaction_get_marked_transaction (transaction_number)== OPERATION_RAPPROCHEE)
						return (my_strdup (_("R")));
					else
						return (NULL);
				}
			}
			break;

		/* mise en place de la pièce comptable */
		case ELEMENT_VOUCHER:
			return (my_strdup (gsb_data_transaction_get_voucher (transaction_number)));

		/* mise en forme des notes */
		case ELEMENT_NOTES:
			return (my_strdup (gsb_data_transaction_get_notes (transaction_number)));

		/* mise en place de l'info banque/guichet */
		case ELEMENT_BANK:
			return (my_strdup (gsb_data_transaction_get_bank_references (transaction_number)));

		/* mise en place du no d'opé */
		case ELEMENT_NO:
			return (utils_str_itoa (transaction_number));

		/* mise en place du no de chèque/virement */
		case ELEMENT_CHQ:
			if (gsb_data_transaction_get_method_of_payment_content (transaction_number))
				return (g_strconcat ("(",
									 gsb_data_transaction_get_method_of_payment_content (transaction_number),
						   			 ")",
						   			 NULL));
			else
				return (NULL);
			break;
    }

	return (NULL);
}

/**
 * update the transaction given in the tree_view
 *
 * \param transaction transaction to update
 *
 * \return FALSE
 **/
gboolean gsb_transactions_list_update_transaction (gint transaction_number)
{
    gint account_number;
	GrisbiWinRun *w_run;

    devel_debug_int (transaction_number);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

    /* first, modify the transaction in the tree view */
    transaction_list_update_transaction (transaction_number);

    /* update the balances */
    transaction_list_set_balances ();

    account_number = gsb_data_transaction_get_account_number (transaction_number);
    gsb_data_account_colorize_current_balance (account_number);

    /* update first page */
    w_run->mise_a_jour_liste_comptes_accueil = TRUE;
    w_run->mise_a_jour_soldes_minimaux = TRUE;
    w_run->mise_a_jour_fin_comptes_passifs = TRUE;

    return FALSE;
}

/**
 * calculate the row_align of the current selected transaction
 * to set it later with gtk_tree_view_scroll_to_cell
 *
 * \param account_number
 *
 * \return a gfloat used as row_align
 **/
gfloat gsb_transactions_list_get_row_align (void)
{
    GtkTreePath *path;
    GtkTreeView *tree_view;
    GdkRectangle back_rect;
    GdkWindow *window;
    gint height_win;
    gfloat row_align;
    gfloat height, rect_y;

    devel_debug (NULL);

    tree_view = GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ());
    path = transaction_list_select_get_path (0);

    if (!path)
	    return 0;

    window = gtk_tree_view_get_bin_window (tree_view);
    if (!gdk_window_is_visible (GDK_WINDOW (window)))
        return 0;

    height_win = gdk_window_get_height (window);

    gtk_tree_view_get_background_area (tree_view, path, NULL, &back_rect);
    gtk_tree_path_free (path);

    /* i didn't find another hack to transform gint to float */
    height = height_win;
    rect_y = back_rect.y;

    row_align = rect_y/height;

	return (row_align);
}

/**
 * set the list at the good position saved before with row_align
 *
 * \param row_align	a gfloat to aligne in the function gtk_tree_view_scroll_to_cell
 * 			if < 0, don't use row_align but let the tree view to move by himself
 * 			to the selected transaction
 *
 * \return FALSE
 **/
gboolean gsb_transactions_list_set_row_align (gfloat row_align)
{
    GtkTreePath *path;
    gint transaction_number;
    gint mother_transaction;
    gint account_number;

    /* devel_debug (NULL); */
    if ((account_number = gsb_gui_navigation_get_current_account ()) == -1)
	    return FALSE;

    /* if we just want to let the tree view by himself, it's here
     * we get the path of the last line in transaction because untill now,
     * we do that when open the form, so only the last line interest us */
    if (row_align < 0)
    {
        path = transaction_list_select_get_path (transaction_list_get_last_line
												 (gsb_data_account_get_nb_rows (account_number)));
        if (path)
        {
            gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
										  path, NULL,
										  FALSE, 0.0, 0.0);
            gtk_tree_path_free (path);
        }

		return FALSE;
    }

    /* ok, we want to use row_align */
    transaction_number = transaction_list_select_get ();

    /* if we are on a child, open the mother */
    mother_transaction = gsb_data_transaction_get_mother_transaction_number (transaction_number);
    if (mother_transaction)
	    gsb_transactions_list_switch_expander (mother_transaction);

    /* if we are on white line, go to the end directly,
     * else at the opening, the white line is hidden */
    if (transaction_number == -1)
    {
        path = transaction_list_select_get_path (transaction_list_get_last_line (
                            gsb_data_account_get_nb_rows (account_number)));
        row_align = 1.0;
    }
    else
	    path = transaction_list_select_get_path (0);

    /* we need to use scroll_to_cell function because at this stade,
     * the tree view is not refreshed so all value with alignment don't work
     * but scroll_to_cell will place us on the good position despite that */
	if (path)
	{
		gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (gsb_transactions_list_get_tree_view ()),
									  path,
									  NULL,
									  FALSE,
									  row_align,
									  0.0);
		gtk_tree_path_free (path);
	}

	return FALSE;
}

/**
 * find column number for the transaction element number
 *
 * \param element_number the element we look for
 *
 * \return column number or -1 if the element is not shown
 **/
gint gsb_transactions_list_find_element_col (gint element_number)
{
    gint i, j;

    for (i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
    {
		for (j=0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
		{
			if (tab_affichage_ope[i][j] == element_number)
				return j;
		}
    }

    return -1;
}

/**
 * find line number for the transaction element number
 *
 * \param element_number the element we look for
 *
 * \return line number or -1 if the element is not shown
 **/
gint gsb_transactions_list_find_element_line (gint element_number)
{
    gint i, j;

    for (i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
    {
		for (j=0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
		{
			if (tab_affichage_ope[i][j] == element_number)
			return i;
		}
    }

    return -1;
}

/**
 * cette fonction calcule le solde de départ pour l'affichage de la première opé
 * du compte
 * c'est soit le solde initial du compte si on affiche les R
 * soit le solde initial - les opés R si elles ne sont pas affichées
 *
 * \param
 * \param
 *
 * \return
 **/
GsbReal gsb_transactions_list_get_solde_debut_affichage (gint account_number,
														 gint floating_point)
{
    GsbReal solde;
    GSList *list_tmp_transactions;

    solde = gsb_data_account_get_init_balance (account_number, floating_point);

    if (gsb_data_account_get_l (account_number) == 0)
        solde = gsb_real_add (solde, gsb_data_archive_store_get_archives_balance (account_number));

    if (gsb_data_account_get_r (account_number))
	    return solde;

    /* the marked R transactions are not showed, add their balance to the initial balance */
    list_tmp_transactions = gsb_data_transaction_get_transactions_list ();

    while (list_tmp_transactions)
    {
        gint transaction_number_tmp;

		transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions->data);

        /* 	si l'opé est ventilée ou non relevée, on saute */
        if (gsb_data_transaction_get_account_number (transaction_number_tmp) == account_number
			&&
            !gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp)
            &&
            gsb_data_transaction_get_marked_transaction (transaction_number_tmp) == OPERATION_RAPPROCHEE)
            solde = gsb_real_add (solde,
								  gsb_data_transaction_get_adjusted_amount (transaction_number_tmp,
																			floating_point));

        list_tmp_transactions = list_tmp_transactions->next;
    }

    return (solde);
}

/**
 * called when the selection changed
 * do some stuff which happen in that case
 *
 * \param new_selected_transaction
 *
 * \return
 **/
void gsb_transactions_list_selection_changed (gint new_selected_transaction)
{
    gint account_number;

    devel_debug_int (new_selected_transaction);

    /* the white number has no account number, so we take the current account */
    if (new_selected_transaction != -1)
    {
        account_number = gsb_data_transaction_get_account_number (new_selected_transaction);
        gsb_menu_set_menus_select_transaction_sensitive (TRUE);

		/* on update le menu de la liste des comptes */
        grisbi_win_menu_move_to_acc_update (TRUE);
    }
    else
    {
        account_number = gsb_gui_navigation_get_current_account ();
        gsb_menu_set_menus_select_transaction_sensitive (FALSE);
    }


    /* save the new current transaction */
    gsb_data_account_set_current_transaction_number (account_number, new_selected_transaction);

    /* show the content of the transaction in the form,
     * only if the form is shown */
    if (grisbi_win_form_expander_is_expanded ())
    {
		GrisbiAppConf *a_conf;

		a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
		if (a_conf->show_transaction_selected_in_form)
            gsb_form_fill_by_transaction (new_selected_transaction, TRUE, FALSE);
        else
            gsb_form_clean (account_number);
    }

    /* give the focus to the transaction_tree_view pbiava 02/09/2009
     * edit due to a regression loss of <CtrlR> */
    if (transactions_tree_view)
        gtk_widget_grab_focus (transactions_tree_view);
}

/**
 * Called to edit a specific transaction
 *
 * \param transaction_number
 *
 * \return FALSE
 **/
gboolean gsb_transactions_list_edit_transaction (gint transaction_number)
{
    devel_debug_int (transaction_number);

    gsb_gui_navigation_set_selection (GSB_ACCOUNT_PAGE,
									  gsb_data_transaction_get_account_number (transaction_number),
                        			  0);
    transaction_list_select (transaction_number);
    gsb_form_fill_by_transaction (transaction_number, TRUE, TRUE);

    return FALSE;
}

/**
 * Called to edit a specific transaction but the number of transaction
 * is passed via a pointer (by g_signal_connect)
 *
 * \param transaction_number a pointer which is the number of the transaction
 *
 * \return FALSE
 **/
void gsb_transactions_list_edit_transaction_by_pointer (gint *transaction_number)
{
    devel_debug_int (GPOINTER_TO_INT (transaction_number));
    gsb_transactions_list_edit_transaction (GPOINTER_TO_INT (transaction_number));
}

/**
 * Called to edit a the current transaction
 *
 * \param
 *
 * \return FALSE
 **/
gboolean gsb_transactions_list_edit_current_transaction (void)
{
    devel_debug (NULL);
    gsb_transactions_list_edit_transaction (gsb_data_account_get_current_transaction_number
											(gsb_gui_navigation_get_current_account ()));
    return FALSE;
}

/**
 * delete a transaction
 * if it's a transfer, delete also the contra-transaction
 * if it's a split, delete the childs
 *
 * \param transaction The transaction to delete
 * \param show_warning TRUE to ask if the user is sure, FALSE directly delete the transaction
 * 			if TRUE, the form will be reset too
 *
 * \return FALSE if canceled or nothing done, TRUE if ok
 **/
gboolean gsb_transactions_list_delete_transaction (gint transaction_number,
												   gint show_warning)
{
    gchar *tmp_str;
    gint account_number;
	GrisbiAppConf *a_conf;
	GrisbiWinRun *w_run;

    devel_debug_int (transaction_number);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

    /* we cannot delete the general white line (-1), but all the others white lines are possibles
     * if show_warning it FALSE (ie this is automatic, and not done by user action */
	if (!transaction_number || transaction_number == -1)
		return FALSE;

    /* if we cannot ask for a white line, so this shouldn't append,
     * if we want to delete a child white line, show_warning must be FALSE (force) */
	if (show_warning && transaction_number < 0)
		return FALSE;

    /* if the transaction is archived, cannot delete it */
    if (gsb_data_transaction_get_archive_number (transaction_number))
    {
		dialogue_error (_("Impossible to delete an archived transaction."));

		return FALSE;
    }

    account_number = gsb_data_transaction_get_account_number (transaction_number);

	/* check if the transaction is not reconciled */
    if (gsb_transactions_list_check_mark (transaction_number))
    {
		gint mother_number;
		gboolean valid_delete = FALSE;

		mother_number = gsb_data_transaction_get_mother_transaction_number (transaction_number);
		if (mother_number && !gsb_data_transaction_get_split_of_transaction (transaction_number))
		{
			valid_delete = TRUE;
		}
		if (!valid_delete)
		{
			dialogue_error (_("Impossible to delete a reconciled transaction.\nThe transaction, "
							  "the contra-transaction or the children if it is a split are "
							  "reconciled. You can remove the reconciliation with Ctrl R if "
							  "it is really necessary."));

			return FALSE;
		}
    }

    /* show a warning */
    if (show_warning)
    {
		const gchar *tmp_struct_name;

		if (gsb_data_transaction_get_mother_transaction_number (transaction_number))
        {
			tmp_struct_name = "delete-child-transaction";
            tmp_str = g_strdup_printf (_("Do you really want to delete the child of the transaction "
										"with party '%s' ?"),
									  gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
															   (transaction_number),
															   FALSE));
        }
        else
        {
			tmp_struct_name = "delete-transaction";
            tmp_str = g_strdup_printf (_("Do you really want to delete transaction with party '%s' ?"),
                         			   gsb_data_payee_get_name (gsb_data_transaction_get_payee_number
																(transaction_number),
																FALSE));
        }
		if (!dialogue_conditional_yes_no_with_items ("tab_delete_msg", tmp_struct_name, tmp_str))
        {
            g_free(tmp_str);

			return FALSE;
        }
        g_free(tmp_str);
    }

    /* move the selection */
	if (transaction_list_select_get () == transaction_number)
    {
		transaction_list_select_down (FALSE);
		gsb_data_account_set_current_transaction_number (account_number,
														 transaction_list_select_get ());
    }

    /* delete the transaction from the tree view,
     * all the children and contra transaction will be removed with that */
    gsb_transactions_list_delete_transaction_from_tree_view (transaction_number);

    /*  update the metatrees, this MUST be before remove_transaction */
    delete_transaction_in_trees (transaction_number);

    /* delete the transaction in memory,
     * all the children and contra transaction will be removed with that */
    gsb_data_transaction_remove_transaction (transaction_number);

    /* update the tree view */
    transaction_list_colorize ();
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	if (a_conf->show_transaction_gives_balance)
        transaction_list_set_color_jour (account_number);
    transaction_list_set_balances ();
	transaction_list_select (gsb_data_account_get_current_transaction_number (account_number));

    /* if we are reconciling, update the amounts */
	if (w_run->equilibrage)
		widget_reconcile_update_amounts ();

    /* we will update the home page */
    w_run->mise_a_jour_liste_comptes_accueil = TRUE;
    w_run->mise_a_jour_soldes_minimaux = TRUE;
    affiche_dialogue_soldes_minimaux ();

    /* We blank form. */
	if (show_warning)
		gsb_form_escape_form ();

    gsb_file_set_modified (TRUE);

	return TRUE;
}

/**
 * Delete the transaction from the tree view
 * if it's a transfer, delete the contra transaction from the tree view
 * if it's a split, delete a the children and if necessary the contra transactions
 *
 * don't do any check about possible or marked... check before
 * do nothing in memory, only on the tree view
 *
 * \param transaction_number
 *
 * \return FALSE
 **/
gboolean gsb_transactions_list_delete_transaction_from_tree_view (gint transaction_number)
{
    gint contra_transaction_number;

    /* devel_debug_int (transaction_number); */
	if (transaction_number == -1)
		return FALSE;

    /* if the transaction is a transfer, erase the contra-transaction */
    contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number);
	if (contra_transaction_number > 0)
		transaction_list_remove_transaction (contra_transaction_number);

    /* check if it's a split, we needn't to erase all splits children, they will be deleted
     * with the mother, but if one of them if a transfer, we need to delete it now */
    if (gsb_data_transaction_get_split_of_transaction (transaction_number))
    {
		GSList *tmp_list;

		tmp_list = gsb_data_transaction_get_transactions_list ();
		while (tmp_list)
		{
			gint test_transaction;

			test_transaction = gsb_data_transaction_get_transaction_number (tmp_list->data);

			contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (test_transaction);

			if (gsb_data_transaction_get_mother_transaction_number (test_transaction) == transaction_number
				&&
				contra_transaction_number > 0)
				transaction_list_remove_transaction (contra_transaction_number);

			tmp_list = tmp_list->next;
		}
    }

    /* now we can just delete the wanted transaction */
    transaction_list_remove_transaction (transaction_number);

	return FALSE;
}

/**
 *  Empty transaction form and select transactions tab.
 *
 * \param
 *
 * \return
 **/
void gsb_transactions_list_select_new_transaction (void)
{
	if (gsb_gui_navigation_get_current_account () == -1)
		return;

    gtk_notebook_set_current_page (GTK_NOTEBOOK (grisbi_win_get_notebook_general ()), 1);
    gsb_form_escape_form();
    gsb_form_show (TRUE);
    transaction_list_select (-1);
    gsb_transactions_list_edit_transaction (-1);
}

/**
 * Remove selected transaction if any.
 *
 * \param
 *
 * \return
 **/
void gsb_transactions_list_remove_transaction (void)
{
	gint current_transaction;

    if (!gsb_transactions_list_assert_selected_transaction())
		return;

	current_transaction = gsb_data_account_get_current_transaction_number (gsb_gui_navigation_get_current_account ());
    gsb_transactions_list_delete_transaction (current_transaction, TRUE);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (grisbi_win_get_notebook_general ()), 1);
}

/**
 * Clone selected transaction if any.  Update user interface as well.
 *
 * \param menu_item
 * \param null
 *
 * \return FALSE
 **/
void gsb_transactions_list_clone_selected_transaction (GtkWidget *menu_item,
													   gpointer null)
{
    gint new_transaction_number;

    if (!gsb_transactions_list_assert_selected_transaction())
        return;

    new_transaction_number = gsb_transactions_list_clone_transaction (
                        gsb_data_account_get_current_transaction_number (
                        gsb_gui_navigation_get_current_account ()),
                        0);

    update_transaction_in_trees (new_transaction_number);

    transaction_list_select (new_transaction_number);
    gsb_transactions_list_edit_transaction (new_transaction_number);
    g_object_set_data (G_OBJECT (gsb_form_get_form_widget ()),
			    "transaction_selected_in_form",
			    GINT_TO_POINTER (-1));

    /* force the update module budget */
    gsb_data_account_set_bet_maj (gsb_gui_navigation_get_current_account (), BET_MAJ_ALL);

    gsb_file_set_modified (TRUE);
}

/**
 * use the current selected transaction as template
 *
 * \param menu_item
 * \param null
 *
 * \return FALSE
 **/
void gsb_transactions_list_clone_template (GtkWidget *menu_item,
										   gpointer null)
{
    gint new_transaction_number;
    gint account_number;
    GDate *date;

    if (!gsb_transactions_list_assert_selected_transaction ())
        return;

    account_number = gsb_gui_navigation_get_current_account ();
    new_transaction_number = gsb_transactions_list_clone_transaction (
                                    gsb_data_account_get_current_transaction_number (
                                    account_number),
								    0);

    date = gdate_today ();
    gsb_data_transaction_set_date (new_transaction_number, date);
    gsb_data_transaction_set_value_date (new_transaction_number, NULL);
    g_date_free (date);

    gsb_transactions_list_update_transaction (new_transaction_number);
    update_transaction_in_trees (new_transaction_number);

    transaction_list_select (new_transaction_number);
    gsb_transactions_list_edit_transaction (new_transaction_number);
    g_object_set_data (G_OBJECT (gsb_form_get_form_widget ()),
			    "transaction_selected_in_form",
			    GINT_TO_POINTER (-1));

    gsb_transactions_list_update_tree_view (account_number, TRUE);

    /* force the update module budget */
    gsb_data_account_set_bet_maj (account_number, BET_MAJ_ALL);

    gsb_file_set_modified (TRUE);
}

/**
 * Move selected transaction to another account.  Normally called as a
 * handler.
 *
 * \param menu_item The GtkMenuItem that triggered this handler.
 * \param
 *
 * \return
 **/
void gsb_transactions_list_move_transaction_to_account_from_menu (gint source_account,
																  gint target_account)
{
	gint current_transaction;

    if (!gsb_transactions_list_assert_selected_transaction ())
		return;

	current_transaction = gsb_data_account_get_current_transaction_number (source_account);
    if (gsb_transactions_list_move_transaction_to_account (current_transaction, target_account))
    {
		gtk_notebook_set_current_page (GTK_NOTEBOOK (grisbi_win_get_notebook_general ()), 1);
		update_transaction_in_trees (gsb_data_account_get_current_transaction_number (source_account)) ;
		gsb_data_account_colorize_current_balance (source_account);
        gsb_file_set_modified (TRUE);
    }
}

/**
 * Convert selected transaction to a template of scheduled transaction
 * via gsb_transactions_list_convert_to_scheduled().
 *
 * \param
 *
 * \return
 **/
void gsb_transactions_list_convert_transaction_to_sheduled (void)
{
    gint scheduled_number;
	GrisbiWinRun *w_run;

    if (!gsb_transactions_list_assert_selected_transaction())
        return;

    scheduled_number = gsb_transactions_list_convert_to_scheduled (gsb_data_account_get_current_transaction_number (
                                    gsb_gui_navigation_get_current_account ()));

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
    w_run->mise_a_jour_liste_echeances_auto_accueil = TRUE;

    if (w_run->equilibrage == FALSE)
    {
        gsb_gui_navigation_set_selection (GSB_SCHEDULER_PAGE, 0, 0);
        gsb_scheduler_list_select (scheduled_number);
        gsb_scheduler_list_edit_transaction (scheduled_number);
    }
    else
        gsb_reconcile_set_last_scheduled_transaction (scheduled_number);

    gsb_file_set_modified (TRUE);
}

/**
 * clone the splitted children of a transaction to splitted children of the scheduled transaction
 *
 * \param transaction_number	mother of the splitted children we look for
 * \param scheduled_number	splitted scheduled transaction we want to add the children
 *
 * \return
 **/
void gsb_transactions_list_splitted_to_scheduled (gint transaction_number,
												  gint scheduled_number)
{
    GSList *list_tmp_transactions;

	list_tmp_transactions = gsb_data_transaction_get_transactions_list ();
    while (list_tmp_transactions)
    {
		gint transaction_number_tmp;

		transaction_number_tmp = gsb_data_transaction_get_transaction_number (list_tmp_transactions->data);

		if (gsb_data_transaction_get_account_number (transaction_number_tmp) == gsb_data_transaction_get_account_number (transaction_number)
			&& gsb_data_transaction_get_mother_transaction_number (transaction_number_tmp) == transaction_number)
		{
			gint split_scheduled_number;

			split_scheduled_number = gsb_data_scheduled_new_scheduled ();
			if (!split_scheduled_number)
				return;

			gsb_data_scheduled_set_account_number (split_scheduled_number,
												   gsb_data_transaction_get_account_number (transaction_number_tmp));
			gsb_data_scheduled_set_date (split_scheduled_number,
										 gsb_data_transaction_get_date (transaction_number_tmp));
			gsb_data_scheduled_set_amount (split_scheduled_number,
										   gsb_data_transaction_get_amount (transaction_number_tmp));
			gsb_data_scheduled_set_currency_number (split_scheduled_number,
													gsb_data_transaction_get_currency_number (transaction_number_tmp));
			gsb_data_scheduled_set_payee_number (split_scheduled_number,
												 gsb_data_transaction_get_payee_number (transaction_number_tmp));
			gsb_data_scheduled_set_category_number (split_scheduled_number,
													gsb_data_transaction_get_category_number (transaction_number_tmp));
			gsb_data_scheduled_set_sub_category_number (split_scheduled_number,
														gsb_data_transaction_get_sub_category_number (transaction_number_tmp));

			/*     pour 1 virement, categ et sous categ sont à 0, et compte_virement contient le no de compte */
			/* 	mais si categ et sous categ sont à 0 et que ce n'est pas un virement, compte_virement = -1 */
			/*     on va changer ça la prochaine version, dès que c'est pas un virement->-1 */
			if (gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp) > 0)
			{
				/* c'est un virement, on met la relation et on recherche le type de la contre opération */

				gint contra_transaction_number;

				gsb_data_scheduled_set_account_number_transfer (split_scheduled_number,
																gsb_data_transaction_get_contra_transaction_account
																(transaction_number_tmp));

				contra_transaction_number = gsb_data_transaction_get_contra_transaction_number (transaction_number_tmp);

				gsb_data_scheduled_set_contra_method_of_payment_number (split_scheduled_number,
																		gsb_data_transaction_get_method_of_payment_number
																		(contra_transaction_number));
			}
			else
				if (!gsb_data_scheduled_get_category_number (split_scheduled_number))
					gsb_data_scheduled_set_account_number_transfer (split_scheduled_number, -1);

			gsb_data_scheduled_set_notes (split_scheduled_number,
										  gsb_data_transaction_get_notes (transaction_number_tmp));
			gsb_data_scheduled_set_method_of_payment_number (split_scheduled_number,
															 gsb_data_transaction_get_method_of_payment_number
															 (transaction_number_tmp));
			gsb_data_scheduled_set_method_of_payment_content (split_scheduled_number,
															  gsb_data_transaction_get_method_of_payment_content
															  (transaction_number_tmp));
			gsb_data_scheduled_set_financial_year_number (split_scheduled_number,
														  gsb_data_transaction_get_financial_year_number
														  (transaction_number_tmp));
			gsb_data_scheduled_set_budgetary_number (split_scheduled_number,
													 gsb_data_transaction_get_budgetary_number (transaction_number_tmp));
			gsb_data_scheduled_set_sub_budgetary_number (split_scheduled_number,
														 gsb_data_transaction_get_sub_budgetary_number (transaction_number_tmp));

			gsb_data_scheduled_set_mother_scheduled_number (split_scheduled_number, scheduled_number);

			/* par défaut, on met en manuel, pour éviter si l'utilisateur se gourre dans la date, */
			/* (c'est le cas, à 0 avec g_malloc0) et que l'opé soit enregistrée immédiatement */

			/* fixes bug : set frequency with frequency of transaction mother */
			gsb_data_scheduled_set_frequency (split_scheduled_number, gsb_data_scheduled_get_frequency (scheduled_number));
		}
	list_tmp_transactions = list_tmp_transactions->next;
    }
}

/**
 * switch the view between show the reconciled transactions or not
 *
 * \param show_r	TRUE or FALSE
 *
 * \return
 **/
void gsb_transactions_list_mise_a_jour_affichage_r (gboolean show_r)
{
    gint current_account;
	GrisbiWinEtat *w_etat;

    devel_debug_int (show_r);
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

    current_account = gsb_gui_navigation_get_current_account ();

    /* show r est déjà positionné on met à jour l'affichage */
    if (show_r == gsb_data_account_get_r (current_account))
    {
        gsb_transactions_list_update_tree_view (current_account, show_r);

        return;
    }

    /*  we check all the accounts */
    /* 	if w_etat->retient_affichage_par_compte is set, only gsb_gui_navigation_get_current_account () will change */
    /* 	else, all the accounts change */

    if (w_etat->retient_affichage_par_compte)
        gsb_data_account_set_r (current_account, show_r);
    else
    {
        GSList *list_tmp;

        list_tmp = gsb_data_account_get_list_accounts ();

        while (list_tmp)
        {
            gint i;

            i = gsb_data_account_get_no_account (list_tmp->data);
            gsb_data_account_set_r (i, show_r);

            list_tmp = list_tmp->next;
        }
    }
	gsb_transactions_list_update_tree_view (current_account, show_r);

    gsb_file_set_modified (TRUE);

    return;
}

/**
 * switch the view between show the reconciled transactions or not
 *
 * \param show_r	TRUE or FALSE
 *
 * \return
 **/
void gsb_transactions_list_show_archives_lines (gboolean show_l)
{
    gint current_account;
	GrisbiWinEtat *w_etat;

    devel_debug_int (show_l);
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

    current_account = gsb_gui_navigation_get_current_account ();

    /*  we check all the accounts */
    /* 	if w_etat->retient_affichage_par_compte is set, only gsb_gui_navigation_get_current_account () will change */
    /* 	else, all the accounts change */

    if (show_l == gsb_data_account_get_l (current_account))
    {
        gsb_transactions_list_update_tree_view (current_account, show_l);
        return;
    }

    gsb_data_account_set_l (current_account, show_l);

    if (!w_etat->retient_affichage_par_compte)
    {
        GSList *list_tmp;

        list_tmp = gsb_data_account_get_list_accounts ();

        while (list_tmp)
        {
            gint i;

            i = gsb_data_account_get_no_account (list_tmp->data);
            gsb_data_account_set_l (i, show_l);

            list_tmp = list_tmp->next;
        }
    }
    gsb_transactions_list_update_tree_view (current_account, show_l);

    gsb_file_set_modified (TRUE);

    return;
}

/**
 * called to change the number of visible rows
 * depends of the conf, change the number of rows of the current account,
 * and if necessary of all the accounts
 * adapts also the tree for the splits to set the expander on the new last line
 *
 * \param rows_number the new number of lines we want to see
 *
 * \return
 **/
void gsb_transactions_list_set_visible_rows_number (gint rows_number)
{
    GSList *list_tmp;
    gint current_account;
	GrisbiWinEtat *w_etat;

    devel_debug_int (rows_number);
	w_etat = (GrisbiWinEtat *) grisbi_win_get_w_etat ();

    current_account = gsb_gui_navigation_get_current_account ();
    if (rows_number == gsb_data_account_get_nb_rows (current_account) && rows_number > 1)
	return;

    /*     we check all the accounts */
    /* 	if w_etat->retient_affichage_par_compte is set, only the current account changes */
    /* 	else, all the accounts change */
    list_tmp = gsb_data_account_get_list_accounts ();

    while (list_tmp)
    {
	gint i;

	i = gsb_data_account_get_no_account (list_tmp->data);

	if (!w_etat->retient_affichage_par_compte
	     ||
	     i == current_account)
	{
	    gsb_data_account_set_nb_rows (i, rows_number);
	}
	list_tmp = list_tmp->next;
    }

    /* we update the screen */
    gsb_transactions_list_update_tree_view (current_account, FALSE);
}

/**
 * check if the transaction given in param should be visible or not
 * according to the account and the current line
 * this function is called to filter the model for each line in transaction_list_filter
 * 	so need to be very fast
 *
 * \param transaction_number 	the pointer in the model, usually a transaction, but can be archive
 * \param account_number	account number
 * \param line_in_transaction	the line in the transaction (0, 1, 2 or 3)
 * \param what_is_line		IS_TRANSACTION / IS_ARCHIVE
 *
 * \return TRUE if the transaction should be shown, FALSE else
 **/
gboolean gsb_transactions_list_transaction_visible (gpointer transaction_ptr,
													gint account_number,
													gint line_in_transaction,
													gint what_is_line)
{
    gint transaction_number;
    gint r_shown;
    gint nb_rows;

    r_shown = gsb_data_account_get_r (account_number);
    nb_rows = gsb_data_account_get_nb_rows (account_number);

    /* first check if it's an archive, if yes and good account, always show it */
    if (what_is_line == IS_ARCHIVE)
    {
        if (gsb_data_account_get_l (account_number))
	        return (gsb_data_archive_store_get_account_number (
                        gsb_data_archive_store_get_number (transaction_ptr)) == account_number);
        else
            return FALSE;
    }

    /* we don't check now for the separator, because it won't be shown if the transaction
     * is not shown, so check the basics for the transaction, and show or not after the separator */

    /*  check now for transactions */
    transaction_number = gsb_data_transaction_get_transaction_number (transaction_ptr);

    /* check the general white line (one for all the list, so no account number) */
    if (transaction_number == -1)
	    return (transaction_list_check_line_is_visible (line_in_transaction, nb_rows));

    /* check the account */
    if (gsb_data_transaction_get_account_number (transaction_number) != account_number)
	return FALSE;

    /* 	    check if it's R and if r is shown */
    if (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
	 &&
	 !r_shown)
	return FALSE;

    /* 	    now we check if we show 1, 2, 3 or 4 lines */
    return transaction_list_check_line_is_visible (line_in_transaction, nb_rows);
}

/**
 * switch the expander of the split given in param
 *
 * \param transaction_number the split we want to switch
 *
 * \return FALSE
 **/
gboolean gsb_transactions_list_switch_expander (gint transaction_number)
{
    GtkTreePath *path;

    devel_debug_int (transaction_number);

    if (!gsb_data_transaction_get_split_of_transaction (transaction_number))
	    return FALSE;

    path = transaction_list_select_get_path (transaction_list_get_last_line (
                            gsb_data_account_get_nb_rows (
                            gsb_gui_navigation_get_current_account ())));

    if (gtk_tree_view_row_expanded (GTK_TREE_VIEW (transactions_tree_view), path))
	    gtk_tree_view_collapse_row (GTK_TREE_VIEW (transactions_tree_view), path);
    else
	    gtk_tree_view_expand_row (GTK_TREE_VIEW (transactions_tree_view), path, FALSE);

    gtk_tree_path_free (path);

    return FALSE;
}

/**
 * replace in the transactions list the archive lines by all the transactions in
 * the archive
 * remove too the archive_store from the archive_store list
 *
 * \param archive_number	the archive to restore
 * \param show_warning		TRUE to show a warning if the R transactions are not shown on the account
 *
 * \return FALSE
 **/
gboolean gsb_transactions_list_restore_archive (gint archive_number,
												gboolean show_warning)
{
    GSList *tmp_list;
    gint account_number;
    gint transaction_number;
    gboolean exist = FALSE;

    devel_debug_int (archive_number);

    /* remove the lines of the archive in the model */
    exist = transaction_list_remove_archive (archive_number);

    /* si l'archive existait bien on ajoute les transactions dans la liste et dans le
     * tree_view. Evite de charger deux fois les données si on supprime l'archive
     * après avoir ajouté les lignes */
    if (exist)
    {
        orphan_child_transactions = NULL;

        /* second step, we add all the archived transactions of that archive into the
         * transactions_list and into the store */
        tmp_list = gsb_data_transaction_get_complete_transactions_list ();
        while (tmp_list)
        {
            transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);
            account_number = gsb_data_transaction_get_account_number (transaction_number);
            if (gsb_data_transaction_get_archive_number (transaction_number) == archive_number
             &&
             gsb_data_archive_store_get_transactions_visibles (archive_number, account_number) == FALSE)
            {
                /* append the transaction to the list of non archived transactions */
                gsb_data_transaction_add_archived_to_list (transaction_number);

                /* the transaction belongs to the archive we want to show, so append it to the list store */
                transaction_list_append_transaction (transaction_number);
            }
            tmp_list = tmp_list->next;
        }

        /* if orphan_child_transactions if filled, there are some children which didn't find their
         * mother, we try again now that all the mothers are in the model */
        if (orphan_child_transactions)
        {
        GSList *orphan_list_copy;

        orphan_list_copy = g_slist_copy (orphan_child_transactions);
        g_slist_free (orphan_child_transactions);
        orphan_child_transactions = NULL;

        tmp_list = orphan_list_copy;
        while (tmp_list)
        {
            transaction_number = GPOINTER_TO_INT (tmp_list->data);
            transaction_list_append_transaction (transaction_number);
            tmp_list = tmp_list->next;
        }
        g_slist_free (orphan_list_copy);

        /* if orphan_child_transactions is not null, there is still some children
         * which didn't find their mother. show them now */
        if (orphan_child_transactions)
        {
            gchar *message = _("Some children didn't find their mother in the list, this shouldn't happen and there is probably a bug behind that. Please contact the Grisbi team.\n\nThe concerned children number are:\n");
            gchar *string_1;
            gchar *string_2;

            string_1 = g_strconcat (message, NULL);
            tmp_list = orphan_child_transactions;
            while (tmp_list)
            {
            string_2 = g_strconcat (string_1,
                         utils_str_itoa (GPOINTER_TO_INT (tmp_list->data)),
                         " - ",
                         NULL);
            g_free (string_1);
            string_1 = string_2;
            tmp_list = tmp_list->next;
            }
            dialogue_warning (string_1);
            g_free (string_1);
        }
        }
    }

    /* remove the structures of archive_model */
    gsb_data_archive_store_remove_by_archive (archive_number);

    /* all the transactions of the archive have been added, we just need to clean the list,
     * but don't touch to the main page and to the current balances... we didn't change anything */
    account_number = gsb_gui_navigation_get_current_account ();
    if (account_number != -1)
        gsb_transactions_list_update_tree_view (account_number, TRUE);

    if (!gsb_data_account_get_r (account_number) && show_warning)
        dialogue (_("You have just recovered an archive, if you don't see any new "
                        "transaction, remember that the R transactions are not showed "
                        "so the archived transactions are certainly hidden...\n\n"
                        "Show the R transactions to make them visible."));

    return FALSE;
}

/**
 * add in the transactions list for the account the transactions in
 * the archive
 * set visible the transactions in the archive_store
 *
 * \param archive_number	the archive to restore
 * \param account_number    the account
 *
 * \return FALSE
 **/
gboolean gsb_transactions_list_add_transactions_from_archive (gint archive_number,
															  gint account_number,
															  gboolean show_warning)
{
    GSList *tmp_list;
    gint transaction_number;
    gboolean exist = FALSE;

    devel_debug_int (archive_number);

    /* remove the line of the archive in the model for the account*/
    exist = transaction_list_remove_archive_line (archive_number, account_number);
    /* set visible the transactions in archive_store */
    gsb_data_archive_store_set_transactions_visibles (archive_number, account_number, TRUE);

    /* si l'archive existait bien on ajoute les transactions dans la liste et dans le
     * tree_view. Evite de charger deux fois les données si on supprime l'archive
     * après avoir ajouté les lignes */
    if (exist)
    {
        orphan_child_transactions = NULL;

        /* second step, we add all the archived transactions of that archive into the
         * transactions_list and into the store */
        tmp_list = gsb_data_transaction_get_complete_transactions_list ();
        while (tmp_list)
        {
            transaction_number = gsb_data_transaction_get_transaction_number (tmp_list->data);

            if (gsb_data_transaction_get_archive_number (transaction_number) == archive_number
             &&
             gsb_data_transaction_get_account_number (transaction_number) == account_number)
            {
                /* append the transaction to the list of non archived transactions */
                gsb_data_transaction_add_archived_to_list (transaction_number);

                /* the transaction belongs to the archive we want to show, so append it to the list store */
                transaction_list_append_transaction (transaction_number);
            }
            tmp_list = tmp_list->next;
        }

        /* if orphan_child_transactions if filled, there are some children which didn't find their
         * mother, we try again now that all the mothers are in the model */
        if (orphan_child_transactions)
        {
            GSList *orphan_list_copy;

            orphan_list_copy = g_slist_copy (orphan_child_transactions);
            g_slist_free (orphan_child_transactions);
            orphan_child_transactions = NULL;

            tmp_list = orphan_list_copy;
            while (tmp_list)
            {
                transaction_number = GPOINTER_TO_INT (tmp_list->data);
                transaction_list_append_transaction (transaction_number);
                tmp_list = tmp_list->next;
            }
            g_slist_free (orphan_list_copy);

            /* if orphan_child_transactions is not null, there is still some children
             * which didn't find their mother. show them now */
            if (orphan_child_transactions)
            {
                gchar *message = _("Some children didn't find their mother in the list, "
                        "this shouldn't happen and there is probably a bug behind that. "
                        "Please contact the Grisbi team.\n\nThe concerned children number are:\n");
                gchar *string_1;
                gchar *string_2;

                string_1 = g_strconcat (message, NULL);
                tmp_list = orphan_child_transactions;
                while (tmp_list)
                {
                    string_2 = g_strconcat (string_1,
                                    utils_str_itoa (GPOINTER_TO_INT (tmp_list->data)),
                                    " - ",
                                    NULL);
                    g_free (string_1);
                    string_1 = string_2;
                    tmp_list = tmp_list->next;
                }
                dialogue_warning (string_1);
                g_free (string_1);
            }
        }

        gsb_transactions_list_update_tree_view (account_number, TRUE);

        if (!gsb_data_account_get_r (account_number) && show_warning)
            dialogue (_("You have just recovered an archive, if you don't see any new "
                            "transaction, remember that the R transactions are not showed "
                            "so the archived transactions are certainly hidden...\n\n"
                            "Show the R transactions to make them visible."));
    }

    gsb_transactions_list_set_visible_archived_button (TRUE);

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_transactions_list_set_largeur_col (void)
{
    gint i;
    gint width;

    for (i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++)
    {
        width = (transaction_col_width[i] * (current_tree_view_width)) / 100;
        if (width > 0)
            gtk_tree_view_column_set_fixed_width (transactions_tree_view_columns[i], width);
    }

    return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *gsb_transactions_list_get_toolbar (void)
{
    return transaction_toolbar;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_transactions_list_change_aspect_liste (gint demande)
{
    switch (demande)
    {
        case 0:
            /* not used */
                return (TRUE);

        /* 	1, 2, 3 et 4 sont les nb de lignes qu'on demande à afficher */
        case 1 :
        case 2 :
        case 3 :
        case 4 :
            gsb_transactions_list_set_visible_rows_number (demande);
            break;

        case 5 :
            /* ope avec r */
            gsb_transactions_list_mise_a_jour_affichage_r (1);
            break;

        case 6 :
            /* ope sans r */
            gsb_transactions_list_mise_a_jour_affichage_r (0);
            break;

        case 7 :
            /* show archive lines */
            gsb_transactions_list_show_archives_lines (1);
            break;

        case 8 :
            /* hide archive lines */
            gsb_transactions_list_show_archives_lines (0);
            break;
    }

    gsb_file_set_modified (TRUE);
    return (TRUE);
}

/**
 * Rend visible ou cache le bouton utilisé pour cacher les transactions archivées
 *
 * \param
 *
 * \return
 **/
void gsb_transactions_list_set_visible_archived_button (gboolean visible)
{
    GtkWidget *button;

    button = g_object_get_data (G_OBJECT (transaction_toolbar), "archived_button");

    gtk_widget_set_visible (button, visible);
}

/**
 * retourne le titre d'une colonne de la liste des opérations.
 *
 *\param numéro de l'élément demandé
 *
 *\return une chaine traduite qui doit être libérée.
 **/
gchar *gsb_transactions_list_get_column_title_from_element (gint element)
{
    if (element < 0)
        return NULL;
    else
        return g_strdup (gettext (labels_titres_colonnes_liste_ope[element]));
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint gsb_transactions_list_get_current_tree_view_width (void)
{
	return current_tree_view_width;
}

/**
 * Initialise le tableau d'affichage des champs de la liste des opérations
 *
 * \param description 	Chaine contenant la liste des champs
 * 						Si NULL utilise les donnes par défaut.
 *
 * \return
 **/
void gsb_transactions_list_init_tab_affichage_ope (const gchar *description)
{
    gint i, j;

	if (description && strlen (description))
	{
		gchar **pointeur_char;

		/* the transactions columns are xx-xx-xx-xx and we want to set in transaction_col_align[1-2-3...] */
		pointeur_char = g_strsplit (description, "-", 0);

		for (i = 0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
		{
			for (j = 0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
			{
				tab_affichage_ope[i][j] = utils_str_atoi (pointeur_char[j + i*CUSTOM_MODEL_VISIBLE_COLUMNS]);
			}
		}
		g_strfreev (pointeur_char);
	}
	else
	{
		for (i = 0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
		{
			for (j = 0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
			{
				tab_affichage_ope[i][j] = tab[i][j];
			}
		}

	}
}

/**
 * Initialise le tableau des alignements de colonne du treeview des transactions
 *
 * \param description 	Chaine contenant la largeurs des colonnes
 * 						Si NULL utilise les donnes par défaut.
 *
 * \return
 **/
void gsb_transactions_list_init_tab_align_col_treeview (const gchar *description)
{
    gint i;

	if (description && strlen (description))
	{
		gchar **pointeur_char;

		/* the transactions columns are xx-xx-xx-xx and we want to set in transaction_col_align[1-2-3...] */
		pointeur_char = g_strsplit (description, "-", 7);

		for (i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++)
			transaction_col_align[i] = utils_str_atoi (pointeur_char[i]);

		g_strfreev (pointeur_char);
    }
	else
	{
		/* defaut value for width of columns */
		for (i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++)
			transaction_col_align[i] = 	transaction_col_align_init[i];
	}
}

/**
 * Initialise le tableau des largeurs de colonne du treeview des transactions
 *
 * \param description 	Chaine contenant la largeurs des colonnes
 * 						Si NULL utilise les donnes par défaut.
 *
 * \return
 **/
void gsb_transactions_list_init_tab_width_col_treeview (const gchar *description)
{
	gint i;

	if (description && strlen (description))
	{
		gchar **pointeur_char;
		gint somme = 0; 			/* calcul du % de la dernière colonne */

		/* the transactions columns are xx-xx-xx-xx and we want to set in transaction_col_width[1-2-3...] */
		pointeur_char = g_strsplit (description, "-", 0);
		if (g_strv_length (pointeur_char) != CUSTOM_MODEL_VISIBLE_COLUMNS)
		{
			/* defaut value for width of columns */
			for (i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++)
				transaction_col_width[i] = transaction_col_width_init[i];
			g_strfreev (pointeur_char);

			return;
		}

		for (i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS -1 ; i++)
		{
			if (strlen ((const gchar *) pointeur_char[i]) == 0)
				transaction_col_width[i] = transaction_col_width_init[i];
			else
				transaction_col_width[i] = utils_str_atoi (pointeur_char[i]);

			somme+= transaction_col_width[i];
		}

		/* si la largeur de la dernière colonne ("Solde") est < 12 */
		/* le chargement des largeurs n'est pas correct */
		/* on diminue la plus grande pour la ramener à 12 pour éviter la réinitialisation */
		if (100 - somme < 12)
		{
			gint diff = 0;
			gint high = 0;
			gint high_j = 0;
			gint j;
			GrisbiWinRun *w_run;

			diff = 12-(100-somme);
			for (j = 0 ; j < i ; j++)
			{
				if (transaction_col_width[j] > high)
				{
					high = transaction_col_width[j];
					high_j = j;
				}
			}
			transaction_col_width[high_j] = high-diff;

			/* à ce stade le fichier n'est pas entièrement chargé */
			/* on ne peut pas utiliser gsb_file_set_modified () */
			w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
			w_run->file_modification = time (NULL);
		}
		else
		{
			transaction_col_width[i] = 100-somme;
		}

		g_strfreev (pointeur_char);
	}
	else
	{
		for (i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS; i++)
		{
			transaction_col_width[i] = transaction_col_width_init[i];
		}
	}
}

/**
 * retourne une chaine formatée des champs de la liste des opérations
 *
 * \param
 *
 * \return a newly allocated chain to be released
 **/
gchar *gsb_transactions_list_get_tab_affichage_ope_to_string (void)
{
    gchar *first_string_to_free;
    gchar *second_string_to_free;
	gchar *tmp_str = NULL;
	gint i = 0;
	gint j = 0;

    for (i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
	{
		for (j=0 ; j< CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
		{
			if (tmp_str)
			{
				first_string_to_free = tmp_str;
				second_string_to_free = utils_str_itoa (tab_affichage_ope[i][j]);
				tmp_str = g_strconcat (first_string_to_free,  "-", second_string_to_free, NULL);
				g_free (first_string_to_free);
				g_free (second_string_to_free);
			}
			else
			{
				tmp_str = utils_str_itoa (tab_affichage_ope[i][j]);
			}
		}
	}

	return tmp_str;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint *gsb_transactions_list_get_tab_align_col_treeview (void)
{
	gint *pointer;

	pointer = transaction_col_align;

	return pointer;
}

/**
 * retourne une chaine formatée des alignements de colonne du treeview transactions
 *
 * \param
 *
 * \return a newly allocated chain to be released
 **/
gchar *gsb_transactions_list_get_tab_align_col_treeview_to_string (void)
{
    gchar *first_string_to_free;
    gchar *second_string_to_free;
	gchar *tmp_str = NULL;
	gint i = 0;

    for (i=0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++)
    {
        if (tmp_str)
        {
			first_string_to_free = tmp_str;
			second_string_to_free = utils_str_itoa (transaction_col_align[i]);
            tmp_str = g_strconcat (first_string_to_free, "-", second_string_to_free,  NULL);
            g_free (first_string_to_free);
            g_free (second_string_to_free);
        }
        else
            tmp_str  = utils_str_itoa (transaction_col_align[i]);
    }

	return tmp_str;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint *gsb_transactions_list_get_tab_width_col_treeview (void)
{
	gint *pointer;

	pointer = transaction_col_width;

	return pointer;
}

/**
 * retourne une chaine formatée des largeurs de colonne du treeview transactions
 *
 * \param
 *
 * \return a newly allocated chain to be released
 **/
gchar *gsb_transactions_list_get_tab_width_col_treeview_to_string (void)
{
    gchar *first_string_to_free;
    gchar *second_string_to_free;
	gchar *tmp_str = NULL;
	gint i = 0;

    for (i=0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++)
    {
        if (tmp_str)
        {
			first_string_to_free = tmp_str;
			second_string_to_free = utils_str_itoa (transaction_col_width[i]);
            tmp_str = g_strconcat (first_string_to_free, "-", second_string_to_free,  NULL);
            g_free (first_string_to_free);
            g_free (second_string_to_free);
        }
        else
            tmp_str  = utils_str_itoa (transaction_col_width[i]);
    }

	return tmp_str;
}

/**
 * retourne un élément du tableau d'affichage des opérations
 *
 * \param	dim_1	première dimension du tableau
 * \param	dim_2 	deuxième dimension du tableau
 *
 * \return 			element number
 **/
gint gsb_transactions_list_get_element_tab_affichage_ope (gint dim_1,
														  gint dim_2)
{
	gint element = 0;

	element = tab_affichage_ope[dim_1][dim_2];

	return element;
}

/**
 *
 *
 * \param			élément à mettre
 * \param	dim_1	première dimension du tableau
 * \param	dim_2 	deuxième dimension du tableau
 *
 * \return
 **/
void gsb_transactions_list_set_element_tab_affichage_ope (gint element_number,
														  gint dim_1,
														  gint dim_2)
{
	tab_affichage_ope[dim_1][dim_2] = element_number;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gint *gsb_transactions_list_get_tab_affichage_ope (void)
{
	gint *pointer;

	pointer = tab_affichage_ope[0];

	return pointer;
}

/**
 * retourne le nom de la colonne
 *
 * \param dim1	row number
 * \param dim1	column number
 *
 * \return		column name must be freed
 **/
gchar *gsb_transactions_list_get_column_title (gint dim_1,
											   gint dim_2)
{
	gchar *column_name = NULL;
	gint element_number = 0;
	gint title_number = 0;

	element_number = tab_affichage_ope[dim_1][dim_2];

	if (element_number)
	{
		title_number = element_number - 1;		/* décalage du tableau des types de champs qui commence à 1 */
		column_name = g_strdup (gettext (labels_titres_colonnes_liste_ope[title_number]));

		return column_name;
	}
	else
		return NULL;
}

/**
 * fill the titres_colonnes_liste_operations variable and the associated tips
 * so get the first row element name and set it for title
 * fill the tips with all the elements of the column
 *
 * \param
 *
 * \return
 **/
void gsb_transactions_list_set_titles_tips_col_list_ope (void)
{
    gchar *row[CUSTOM_MODEL_VISIBLE_COLUMNS];
    gint i;
    gint j;

    /* unset the titles and tips */
	gsb_transactions_list_free_titles_tips_col_list_ope ();

    for (i=0 ; i<TRANSACTION_LIST_ROWS_NB ; i++)
	for (j=0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
	{
		row[j] = gsb_transactions_list_get_column_title (i, j);

		/* on the first row, set for titles and tips, for others row, only for tips */
	    if (i)
	    {
            if (row[j])
            {
                if (tips_col_liste_operations[j])
				{
					gchar *free_str;

					free_str = tips_col_liste_operations[j];
                    tips_col_liste_operations[j] = g_strconcat (tips_col_liste_operations[j],
                                                    "- ",
                                                    row[j], " ",
                                                    NULL);
					g_free (free_str);
				}
                else
                    tips_col_liste_operations[j] = g_strconcat (" ", row[j], " ", NULL);

                if (!titres_colonnes_liste_operations[j])
                    titres_colonnes_liste_operations[j] = row[j];
                else
                    g_free (row[j]);
            }
	    }
	    else
	    {
            if (row[j])
            {
                titres_colonnes_liste_operations[j] = row[j];
                tips_col_liste_operations[j] = g_strconcat (" ", row[j], " ", NULL);
            }
	    }
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_transactions_list_free_titles_tips_col_list_ope (void)
{
    gint j;

    for (j=0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++)
    {
		if (titres_colonnes_liste_operations[j])
			g_free (titres_colonnes_liste_operations[j]);
        titres_colonnes_liste_operations[j] = NULL;

		if (tips_col_liste_operations[j])
			g_free (tips_col_liste_operations[j]);
        tips_col_liste_operations[j] = NULL;
    }
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void gsb_transactions_list_show_menu_import_rule (gint account_number)
{
	gboolean show_menu_import_rules;

	show_menu_import_rules = gsb_data_import_rule_account_has_rule (account_number);
	if (show_menu_import_rules)
		gtk_widget_show (menu_import_rules);
    else
        gtk_widget_hide (menu_import_rules);
}

void gsb_transactions_list_set_current_tree_view_width (gint new_tree_view_width)
{
	current_tree_view_width = new_tree_view_width;
}
/**
 *
 *
 * \param menu_item
 * \param transaction number
 *
 * \return FALSE
 **/
void gsb_transactions_list_search (GtkWidget *menu_item,
								   gint *transaction_number)
{
	WidgetSearchTransaction *search;
	GrisbiWin *win;
	gint result;

	win = grisbi_app_get_active_window (NULL);
	search = widget_search_transaction_new (GTK_WIDGET (win), transaction_number);
	gtk_window_present (GTK_WINDOW (search));
	gtk_widget_show_all (GTK_WIDGET (search));
	result = gtk_dialog_run (GTK_DIALOG (search));
	widget_search_transaction_dialog_response (GTK_DIALOG (search), result);
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
