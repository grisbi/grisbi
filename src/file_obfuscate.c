/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/* 			http://www.grisbi.org				      */
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
 * \file file_obfuscate.c
 * anonymyze the file to send with bugreports
 */


#include "include.h"

/*START_INCLUDE*/
#include "file_obfuscate.h"
#include "./dialog.h"
#include "./gsb_assistant.h"
#include "./gsb_data_account.h"
#include "./gsb_data_bank.h"
#include "./gsb_data_budget.h"
#include "./gsb_data_category.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_reconcile.h"
#include "./gsb_data_report.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_file_save.h"
#include "./gsb_file_util.h"
#include "./gsb_status.h"
#include "./utils_files.h"
#include "./include.h"
/*END_INCLUDE*/


/*START_STATIC*/
static GtkWidget *file_obfuscate_page_1 ( void );
static GtkWidget *file_obfuscate_page_2 ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern gchar *nom_fichier_comptes ;
extern gsb_real null_real ;
/*END_EXTERN*/


static GtkWidget *button_accounts_details;
static GtkWidget *button_amount;
static GtkWidget *button_accounts_names;
static GtkWidget *button_payee;
static GtkWidget *button_categories;
static GtkWidget *button_budgets;
static GtkWidget *button_notes;
static GtkWidget *button_banks;
static GtkWidget *button_reports;
static GtkWidget *button_reconcile;

/**
 * called by menubar to obfuscate the file
 *
 * \param
 *
 * \return TRUE
 * */
gboolean file_obfuscate_run ( void )
{
    GtkWidget *assistant;
    gint result;

    gsb_status_message ( _("Obfuscate file...") );

    assistant = gsb_assistant_new ( _("Grisbi file obfuscate"),
				    _("This assistant produces anonymized copies of account files, with all personnal data replaced with harmless random data.\n"
				      "You can then send us account files that hold the same structure than yours but without personnal informations.\n"
				      "Still, check that bugs you submit are still valid with anonymized files.\n"
					  "\n"
				      "To avoid any problems in your file, after saving the modified file,\n"
				      "Grisbi will close close without letting you saving anything,\n"
				      "so if you didn't save your changes, please stop this assistant,\n"
				      "save your work and launch again the obfuscate creation" ),
				    "bug.png",
				    NULL );

    gsb_assistant_add_page ( assistant,
			     file_obfuscate_page_1 (),
			     1, 0, 2, NULL ); 
    gsb_assistant_add_page ( assistant,
			     file_obfuscate_page_2 (),
			     2, 1, -1, NULL ); 
    
    result = gsb_assistant_run ( assistant );

    if (result == GTK_RESPONSE_APPLY)
    {
	/* obfuscate the file */
	GSList *tmp_list;
	gchar *filename;
	
	/*  remove the swp file */
	gsb_file_util_modify_lock (FALSE);

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_accounts_details)))
	{
	    /* hide the details of account but not the names */
	    tmp_list = gsb_data_account_get_list_accounts ();
	    while (tmp_list)
	    {
		gint account_number = gsb_data_account_get_no_account (tmp_list -> data);
		
		gsb_data_account_set_id (account_number,
					 g_strdup_printf ("id account %d", account_number));
		gsb_data_account_set_comment (account_number, NULL);
		gsb_data_account_set_holder_name (account_number, NULL);
		gsb_data_account_set_holder_address (account_number, NULL);
		gsb_data_account_set_init_balance (account_number, null_real);
		gsb_data_account_set_mini_balance_wanted (account_number, null_real);
		gsb_data_account_set_mini_balance_authorized (account_number, null_real);
		gsb_data_account_set_current_balance (account_number, null_real);
		gsb_data_account_set_marked_balance (account_number, null_real);
		gsb_data_account_set_bank_branch_code (account_number, NULL);
		gsb_data_account_set_bank_account_number (account_number, NULL);
		gsb_data_account_set_bank_account_key (account_number, NULL);

		tmp_list = tmp_list -> next;
	    }
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_amount)))
	{
	    /* hide the amounts of transactions */
	    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
	    while (tmp_list)
	    {
		gint transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

		gsb_data_transaction_set_amount (transaction_number, null_real);
		gsb_data_transaction_set_voucher (transaction_number, NULL);
		gsb_data_transaction_set_bank_references (transaction_number, NULL);

		tmp_list = tmp_list -> next;
	    }

	    /* hide the amounts of scheduled transactions */
	    tmp_list = gsb_data_scheduled_get_scheduled_list ();
	    while (tmp_list)
	    {
		gint scheduled_number = gsb_data_scheduled_get_scheduled_number (tmp_list -> data);

		gsb_data_scheduled_set_amount (scheduled_number, null_real);
		tmp_list = tmp_list -> next;
	    }
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_accounts_names)))
	{
	    /* hide the accounts names */
	    tmp_list = gsb_data_account_get_list_accounts ();
	    while (tmp_list)
	    {
		gint account_number = gsb_data_account_get_no_account (tmp_list -> data);
		
		gsb_data_account_set_name (account_number,
					   g_strdup_printf ("Account n°%d", account_number));

		tmp_list = tmp_list -> next;
	    }
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_payee)))
	{
	    /* hide the payees names */
	    tmp_list = gsb_data_payee_get_payees_list ();
	    while (tmp_list)
	    {
		gint payee_number = gsb_data_payee_get_no_payee (tmp_list -> data);

		gsb_data_payee_set_name (payee_number,
					 g_strdup_printf ( "Payee n°%d", payee_number));
		gsb_data_payee_set_description (payee_number, NULL);

		tmp_list = tmp_list -> next;
	    }
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_categories)))
	{
	    /* hide the categories */
	    tmp_list = gsb_data_category_get_categories_list ();
	    while (tmp_list)
	    {
		GSList *list_sub_categ;
		gint category_number = gsb_data_category_get_no_category (tmp_list -> data);

		gsb_data_category_set_name (category_number,
					    g_strdup_printf ( "Category n°%d", category_number));

		list_sub_categ = gsb_data_category_get_sub_category_list (category_number);
		while (list_sub_categ)
		{
		    gint sub_categ_number = gsb_data_category_get_no_sub_category (list_sub_categ -> data);

		    gsb_data_category_set_sub_category_name (category_number, sub_categ_number,
							     g_strdup_printf ("Sub-category n°%d", sub_categ_number));
		    list_sub_categ = list_sub_categ -> next;
		}
		tmp_list = tmp_list -> next;
	    }
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_budgets)))
	{
	    /* hide the budgets */
	    tmp_list = gsb_data_budget_get_budgets_list ();
	    while (tmp_list)
	    {
		GSList *list_sub_budget;
		gint budget_number = gsb_data_budget_get_no_budget (tmp_list -> data);

		gsb_data_budget_set_name (budget_number,
					  g_strdup_printf ( "Budget n°%d", budget_number));

		list_sub_budget = gsb_data_budget_get_sub_budget_list (budget_number);
		while (list_sub_budget)
		{
		    gint sub_budget_number = gsb_data_budget_get_no_sub_budget (list_sub_budget -> data);

		    gsb_data_budget_set_sub_budget_name (budget_number, sub_budget_number,
							 g_strdup_printf ("Sub-budget n°%d", sub_budget_number));
		    list_sub_budget = list_sub_budget -> next;
		}
		tmp_list = tmp_list -> next;
	    }
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_notes)))
	{
	    /* hide the notes */
	    tmp_list = gsb_data_transaction_get_complete_transactions_list ();
	    while (tmp_list)
	    {
		gint transaction_number = gsb_data_transaction_get_transaction_number (tmp_list -> data);

		gsb_data_transaction_set_notes (transaction_number, NULL);

		tmp_list = tmp_list -> next;
	    }

	    /* hide the notes of scheduled transactions */
	    tmp_list = gsb_data_scheduled_get_scheduled_list ();
	    while (tmp_list)
	    {
		gint scheduled_number = gsb_data_scheduled_get_scheduled_number (tmp_list -> data);

		gsb_data_scheduled_set_notes (scheduled_number, NULL);
		tmp_list = tmp_list -> next;
	    }
	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_banks)))
	{
	    /* hide the banks */
	    tmp_list = gsb_data_bank_get_bank_list ();
	    while (tmp_list)
	    {
		gint bank_number = gsb_data_bank_get_no_bank (tmp_list -> data);

		gsb_data_bank_set_name (bank_number,
					g_strdup_printf ("Bank n°%d", bank_number));
		gsb_data_bank_set_code (bank_number, NULL);
		gsb_data_bank_set_bank_address (bank_number, NULL);
		gsb_data_bank_set_bank_tel (bank_number, NULL);
		gsb_data_bank_set_bank_mail (bank_number, NULL);
		gsb_data_bank_set_bank_web (bank_number, NULL);
		gsb_data_bank_set_bank_note (bank_number, NULL);
		gsb_data_bank_set_correspondent_name (bank_number, NULL);
		gsb_data_bank_set_correspondent_tel (bank_number, NULL);
		gsb_data_bank_set_correspondent_mail (bank_number, NULL);
		gsb_data_bank_set_correspondent_fax (bank_number, NULL);

		tmp_list = tmp_list -> next;
	    }


	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_reports)))
	{
	    /* hide the reports names*/
	    tmp_list = gsb_data_report_get_report_list ();
	    while (tmp_list)
	    {
		gint report_number = gsb_data_report_get_report_number (tmp_list -> data);

		gsb_data_report_set_report_name ( report_number, 
						  g_strdup_printf ( "Report n°%d", report_number));

		tmp_list = tmp_list -> next;
	    }


	}

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button_reconcile)))
	{
	    /* hide the reconciles */
	    GList *reconcile_list;
	    reconcile_list = gsb_data_reconcile_get_reconcile_list ();
	    while (reconcile_list)
	    {
		gint reconcile_number = gsb_data_reconcile_get_no_reconcile (reconcile_list -> data);

		gsb_data_reconcile_set_init_balance (reconcile_number, null_real);
		gsb_data_reconcile_set_final_balance (reconcile_number, null_real);

		reconcile_list = reconcile_list -> next;
	    }
	}

	if (nom_fichier_comptes)
	{
	    /* remove the .gsb */
	    nom_fichier_comptes[strlen(nom_fichier_comptes) -4] = 0;
	    filename = g_strconcat ( nom_fichier_comptes, "-obfuscate.gsb", NULL);
	}
	else
	    filename = g_strconcat ( my_get_gsb_file_default_dir (), "No_name-obfuscate", NULL);

	if (gsb_file_save_save_file (filename, FALSE, FALSE))
	    dialogue_warning (g_strdup_printf (_("Obfuscate file saved as %s"),
					       filename ));
	else
	    dialogue_error (g_strdup_printf (_("Grisbi couldn't save the file %s"),
					     filename ));

	/* bye bye */
	exit (0);
    }

    gtk_widget_destroy ( assistant );
    
    return FALSE;
}

/**
 * first page of the assistant
 * to select what we want to anonymyze
 *
 * \param
 *
 * \return the new page
 * */
GtkWidget *file_obfuscate_page_1 ( void )
{
    GtkWidget *vbox;
    GtkWidget *label;

    vbox = gtk_vbox_new (FALSE, 5);

    label = gtk_label_new (_("Select the parts to hide :"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 10);
    label = gtk_label_new (_("(you may want to keep something to reproduce the bug or find it easilly)"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 10);
    label = gtk_label_new (NULL);
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 10);
    
    button_accounts_details = gtk_check_button_new_with_label (_("Hide the accounts details"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_accounts_details,
			 FALSE, FALSE, 5);

    button_amount = gtk_check_button_new_with_label (_("Hide the amounts"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_amount,
			 FALSE, FALSE, 5);

    button_accounts_names = gtk_check_button_new_with_label (_("Hide the accounts names"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_accounts_names,
			 FALSE, FALSE, 5);

    button_payee = gtk_check_button_new_with_label (_("Hide the payees"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_payee,
			 FALSE, FALSE, 5);

    button_categories = gtk_check_button_new_with_label (_("Hide the categories"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_categories,
			 FALSE, FALSE, 5);

    button_budgets = gtk_check_button_new_with_label (_("Hide the budgets"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_budgets,
			 FALSE, FALSE, 5);

    button_notes = gtk_check_button_new_with_label (_("Hide the notes"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_notes,
			 FALSE, FALSE, 5);

    button_banks = gtk_check_button_new_with_label (_("Hide the banks"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_banks,
			 FALSE, FALSE, 5);

    button_reports = gtk_check_button_new_with_label (_("Hide the reports names"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_reports,
			 FALSE, FALSE, 5);

    button_reconcile = gtk_check_button_new_with_label (_("Hide the reconciles names and amounts"));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button_reconcile,
			 FALSE, FALSE, 5);

    gtk_widget_show_all (vbox);
    return vbox;
}

/**
 * second page of the assistant
 *
 * \param
 *
 * \return the new page
 * */
GtkWidget *file_obfuscate_page_2 ( void )
{
    GtkWidget *vbox;
    GtkWidget *label;

    vbox = gtk_vbox_new (FALSE, 5);

    label = gtk_label_new (_("Please press the 'Close' button to obfuscate the file\n"
			     "The name will be the same of your file with -obfuscate after, in the same directory\n"
			     "Please check the bug is still there and send your file with the explanation to\n"
			     "make the bug at bugsreports@listes.grisbi.org or on the bugracker (Mantis)\n"
			     "at http://grisbi.tuxfamily.org/mantis/main_page.php\n\n"
			     "The account is saved in text, you can check with a text editor if there are still no information in your file\n"
			     "Thanks !\n\n"
			     "Grisbi will close immediatly after saving the obfuscated file" ));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 10);

    gtk_widget_show_all (vbox);
    return vbox;
}

