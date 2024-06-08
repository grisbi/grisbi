/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/* 			https://www.grisbi.org				      */
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


#include "config.h"

#include "include.h"
#include <stdlib.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "file_obfuscate.h"
#include "bet_data.h"
#include "dialog.h"
#include "grisbi_win.h"
#include "gsb_assistant.h"
#include "gsb_data_account.h"
#include "gsb_data_bank.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_payee.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_report.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_dirs.h"
#include "gsb_file_save.h"
#include "gsb_file_util.h"
#include "structures.h"
#include "utils.h"
/*END_INCLUDE*/


/*START_STATIC*/
static GtkWidget *file_obfuscate_page_1 ( void );
static GtkWidget *file_obfuscate_page_2 ( void );
/*END_STATIC*/

/*START_EXTERN*/
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

    grisbi_win_status_bar_message ( _("Obfuscating file...") );

    assistant = gsb_assistant_new ( _("Grisbi file obfuscation"),
                        _("This assistant produces anonymized copies of account files, with "
                        "all personal data replaced with harmless random data, in order to "
                        "attach an anonimized copy of your Grisbi file with any bug report "
                        "you submit."
                        "\n\n"
                        "That said, please check that bugs you submit are still valid with "
                        "anonymized version of your files.\n"
                        "\n"
                        "To avoid any problems in your file, after saving the modified file, "
                        "Grisbi will close without letting you saving anything.  "
                        "So if you didn't save your changes, please stop this assistant, "
                        "save your work and restart the obfuscation process.\n\n"
                        "In next page, you will be able to select individual features to "
                        "obfuscate or to keep depending on the level of privacy needed."),
                        "gsb-bug-32.png",
                        NULL );

    gsb_assistant_add_page ( assistant, file_obfuscate_page_1 (), 1, 0, 2, NULL );
    gsb_assistant_add_page ( assistant, file_obfuscate_page_2 (), 2, 1, -1, NULL );

    result = gsb_assistant_run ( assistant );

    if ( result == GTK_RESPONSE_APPLY )
    {
        /* obfuscate the file */
        GSList *tmp_list;
		gchar *filename;
        gchar *nom_fichier_comptes;

		/* on récupère d'abord le nom du fichier */
		nom_fichier_comptes = g_strdup (grisbi_win_get_filename (NULL));
        /*  remove the swp file */
        gsb_file_util_modify_lock (nom_fichier_comptes, FALSE);

        /* hide the accounts data */
        tmp_list = gsb_data_account_get_list_accounts ();
        while ( tmp_list )
        {
            gint account_number = gsb_data_account_get_no_account ( tmp_list -> data );

            /* hide the IBAN number */
            gsb_data_account_set_bank_account_iban ( account_number, NULL );

            if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_accounts_details ) ) )
            {
                /* hide the details of account but not the names */
                gchar *tmp_str;

                tmp_str = g_strdup_printf ( "id account %d", account_number );
                gsb_data_account_set_id ( account_number, tmp_str );
                g_free ( tmp_str );

                gsb_data_account_set_comment (account_number, NULL);
                gsb_data_account_set_holder_name (account_number, NULL);
                gsb_data_account_set_holder_address (account_number, NULL);
                gsb_data_account_set_init_balance (account_number, null_real);
                gsb_data_account_set_mini_balance_wanted (account_number, null_real);
                gsb_data_account_set_mini_balance_authorized (account_number, null_real);
                gsb_data_account_set_bank_branch_code (account_number, NULL);
                gsb_data_account_set_bank_account_number (account_number, NULL);
                gsb_data_account_set_bank_account_key (account_number, NULL);
            }

            if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_accounts_names ) ) )
            {
                /* hide the accounts names */
                gchar *tmp_str;

                tmp_str = g_strdup_printf ( "Account n°%d", account_number );
                gsb_data_account_set_name ( account_number, tmp_str );
                g_free ( tmp_str );

            }

            /* hide the budgetary data */
            if ( gsb_data_account_get_bet_use_budget ( account_number ) )
            {
                /* hide the historiques data */
                gint bet_hist_data;

                bet_hist_data = gsb_data_account_get_bet_hist_data ( account_number );

                if ( bet_hist_data == 0 && gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_categories ) ) )
                {
                    bet_data_hist_reset_all_amounts ( account_number );
                }
                if ( bet_hist_data == 1 && gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_budgets ) ) )
                {
                    bet_data_hist_reset_all_amounts ( account_number );
                }
                gsb_data_account_set_bet_finance_capital ( account_number, 0.0 );
                gsb_data_account_set_bet_finance_taux_annuel ( account_number, 0.0 );
                gsb_data_account_set_bet_finance_frais ( account_number, 0.0 );
                gsb_data_account_set_bet_finance_capital ( account_number, 0.0 );
                gsb_data_account_set_bet_months ( account_number, 0 );
            }

            tmp_list = tmp_list -> next;
        }

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_accounts_names ) ) )
        {
            /* hide the partial balance accounts names */
            tmp_list = gsb_data_partial_balance_get_list ();
            while ( tmp_list )
            {
                gint partial_balance_number;
                gchar *tmp_str;

                partial_balance_number = gsb_data_partial_balance_get_number ( tmp_list->data );
                tmp_str = g_strdup_printf ( "Partial balance n°%d", partial_balance_number );
                gsb_data_partial_balance_set_name ( partial_balance_number, tmp_str );
                g_free ( tmp_str );

                tmp_list = tmp_list->next;
            }
        }

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_amount ) ) )
        {
            /* hide the amounts of transactions */
            tmp_list = gsb_data_transaction_get_complete_transactions_list ();
            while ( tmp_list )
            {
                gint transaction_number;

                transaction_number = gsb_data_transaction_get_transaction_number ( tmp_list -> data );
                gsb_data_transaction_set_amount ( transaction_number, null_real );
                gsb_data_transaction_set_voucher ( transaction_number, NULL );
                gsb_data_transaction_set_bank_references ( transaction_number, NULL );

                tmp_list = tmp_list -> next;
            }

            /* hide the amounts of scheduled transactions */
            tmp_list = gsb_data_scheduled_get_scheduled_list ();
            while ( tmp_list )
            {
                gint scheduled_number;

                scheduled_number = gsb_data_scheduled_get_scheduled_number ( tmp_list -> data );
                gsb_data_scheduled_set_amount ( scheduled_number, null_real );
                tmp_list = tmp_list -> next;
            }
        }

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_payee ) ) )
        {
            /* hide the payees names */
            tmp_list = gsb_data_payee_get_payees_list ();
            while ( tmp_list )
            {
            gint payee_number;
            gchar *tmp_str;

            payee_number = gsb_data_payee_get_no_payee ( tmp_list -> data );
            tmp_str = g_strdup_printf ( "Payee n°%d", payee_number );
            gsb_data_payee_set_name ( payee_number, tmp_str );
            g_free ( tmp_str );

            gsb_data_payee_set_description ( payee_number, NULL );
            gsb_data_payee_set_search_string ( payee_number, NULL );

            tmp_list = tmp_list -> next;
            }
        }

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_categories ) ) )
        {
            /* hide the categories */
            tmp_list = gsb_data_category_get_categories_list ();
            while ( tmp_list )
            {
                GSList *list_sub_categ;
                gint category_number;
                gchar *tmp_str;

                category_number = gsb_data_category_get_no_category ( tmp_list -> data );
                tmp_str = g_strdup_printf ( "Category n°%d", category_number );
                gsb_data_category_set_name ( category_number, tmp_str );
                g_free ( tmp_str );

                list_sub_categ = gsb_data_category_get_sub_category_list ( category_number );
                while ( list_sub_categ )
                {
                    gint sub_categ_number;

                    sub_categ_number = gsb_data_category_get_no_sub_category ( list_sub_categ -> data );
                    tmp_str = g_strdup_printf ( "Sub-category n°%d", sub_categ_number );
                    gsb_data_category_set_sub_category_name ( category_number, sub_categ_number, tmp_str );
                    g_free ( tmp_str );

                    list_sub_categ = list_sub_categ -> next;
                }
                tmp_list = tmp_list -> next;
            }
        }

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_budgets ) ) )
        {
            /* hide the budgets */
            tmp_list = gsb_data_budget_get_budgets_list ();
            while ( tmp_list )
            {
                GSList *list_sub_budget;
                gint budget_number;
                gchar *tmp_str;

                budget_number = gsb_data_budget_get_no_budget ( tmp_list -> data );
                tmp_str = g_strdup_printf ( "Budget n°%d", budget_number );
                gsb_data_budget_set_name ( budget_number, tmp_str );
                g_free ( tmp_str );

                list_sub_budget = gsb_data_budget_get_sub_budget_list (budget_number);
                while ( list_sub_budget )
                {
                    gint sub_budget_number;

                    sub_budget_number = gsb_data_budget_get_no_sub_budget (list_sub_budget -> data);
                    tmp_str = g_strdup_printf ( "Sub-budget n°%d", sub_budget_number );
                    gsb_data_budget_set_sub_budget_name ( budget_number, sub_budget_number, tmp_str );
                    g_free ( tmp_str );

                    list_sub_budget = list_sub_budget -> next;
                }
                tmp_list = tmp_list -> next;
            }
        }

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_notes ) ) )
        {
            /* hide the notes */
            tmp_list = gsb_data_transaction_get_complete_transactions_list ();
            while ( tmp_list )
            {
                gint transaction_number;

                transaction_number = gsb_data_transaction_get_transaction_number ( tmp_list -> data );
                gsb_data_transaction_set_notes ( transaction_number, NULL );

                tmp_list = tmp_list -> next;
            }

            /* hide the notes of scheduled transactions */
            tmp_list = gsb_data_scheduled_get_scheduled_list ();
            while ( tmp_list )
            {
                gint scheduled_number;

                scheduled_number = gsb_data_scheduled_get_scheduled_number ( tmp_list -> data );
                gsb_data_scheduled_set_notes (scheduled_number, NULL);
                tmp_list = tmp_list -> next;
            }
        }

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_banks ) ) )
        {
            /* hide the banks */
            tmp_list = gsb_data_bank_get_bank_list ();
            while ( tmp_list )
            {
                gint bank_number;
                gchar *tmp_str;

                bank_number = gsb_data_bank_get_no_bank ( tmp_list -> data );
                tmp_str = g_strdup_printf ( "Bank n°%d", bank_number );
                gsb_data_bank_set_name ( bank_number, tmp_str );
                g_free ( tmp_str );

                gsb_data_bank_set_code ( bank_number, NULL );
                gsb_data_bank_set_bic ( bank_number, NULL );
                gsb_data_bank_set_bank_address ( bank_number, NULL );
                gsb_data_bank_set_bank_tel ( bank_number, NULL );
                gsb_data_bank_set_bank_mail ( bank_number, NULL );
                gsb_data_bank_set_bank_web ( bank_number, NULL );
                gsb_data_bank_set_bank_note ( bank_number, NULL );
                gsb_data_bank_set_correspondent_name ( bank_number, NULL );
                gsb_data_bank_set_correspondent_tel ( bank_number, NULL );
                gsb_data_bank_set_correspondent_mail ( bank_number, NULL );
                gsb_data_bank_set_correspondent_fax ( bank_number, NULL );

                tmp_list = tmp_list -> next;
            }
        }

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_reports ) ) )
        {
            /* hide the reports names*/
            tmp_list = gsb_data_report_get_report_list ();
            while ( tmp_list )
            {
                gint report_number;
                gchar *tmp_str;

                report_number = gsb_data_report_get_report_number ( tmp_list -> data );
                tmp_str = g_strdup_printf ( "Report n°%d", report_number );
                gsb_data_report_set_report_name ( report_number, tmp_str );
                g_free ( tmp_str );

                tmp_list = tmp_list -> next;
            }
        }

        if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button_reconcile ) ) )
        {
            /* hide the reconciliations */
            GList *reconcile_list;
            reconcile_list = gsb_data_reconcile_get_reconcile_list ();
            while ( reconcile_list )
            {
                gint reconcile_number;

                reconcile_number = gsb_data_reconcile_get_no_reconcile (reconcile_list -> data);
                gsb_data_reconcile_set_init_balance (reconcile_number, null_real);
                gsb_data_reconcile_set_final_balance (reconcile_number, null_real);

                reconcile_list = reconcile_list -> next;
            }
        }

        if ( nom_fichier_comptes )
        {
            /* remove the .gsb */
            nom_fichier_comptes[strlen ( nom_fichier_comptes ) -4] = '\0';
            filename = g_strconcat ( nom_fichier_comptes, "-obfuscated.gsb", NULL);
        }
        else
            filename = g_build_filename ( gsb_dirs_get_default_dir (), "No_name-obfuscated.gsb", NULL);

        if ( gsb_file_save_save_file ( filename, FALSE,0 ) )
            dialogue_hint ( g_strdup_printf ( _("Obfuscated file saved as\n'%s'"), filename ),
                        _("Obfuscation succeeded") );
        else
            dialogue_error_hint (g_strdup_printf (_("Grisbi couldn't save the file\n'%s'"), filename ),
                     _("Obfuscation failed") );

        g_free ( filename );
		g_free (nom_fichier_comptes);

        /* bye bye */
        exit (0);
    }

    gtk_widget_destroy ( assistant );
    grisbi_win_status_bar_message ( _("Done.") );

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
    GtkWidget *vbox, *paddingbox;
    GtkWidget *button_everything;

    vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), BOX_BORDER_WIDTH );

    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Select features to hide :\n") );

    button_everything = gtk_check_button_new_with_label ( _("Hide everything") );
    g_signal_connect ( button_everything, "toggled", G_CALLBACK ( radio_set_active_linked_widgets ), NULL );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_everything, FALSE, FALSE, 0 );

    button_accounts_names = gtk_check_button_new_with_label ( _("Hide accounts names") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_accounts_names, FALSE, FALSE, 0 );
    register_button_as_linked ( button_everything, button_accounts_names );

    button_accounts_details = gtk_check_button_new_with_label ( _("Hide accounts details") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_accounts_details, FALSE, FALSE, 0 );
    register_button_as_linked ( button_everything, button_accounts_details );

    button_amount = gtk_check_button_new_with_label ( _("Hide amounts") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_amount, FALSE, FALSE, 0 );
    register_button_as_linked ( button_everything, button_amount );

    button_payee = gtk_check_button_new_with_label ( _("Hide payees names") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_payee, FALSE, FALSE, 0 );
    register_button_as_linked ( button_everything, button_payee );

    button_categories = gtk_check_button_new_with_label ( _("Hide categories names") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_categories, FALSE, FALSE, 0 );
    register_button_as_linked ( button_everything, button_categories );

    button_budgets = gtk_check_button_new_with_label ( _("Hide budgets names") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_budgets, FALSE, FALSE, 0 );
    register_button_as_linked ( button_everything, button_budgets );

    button_notes = gtk_check_button_new_with_label ( _("Hide notes") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_notes, FALSE, FALSE, 0 );
    register_button_as_linked ( button_everything, button_notes );

    button_banks = gtk_check_button_new_with_label ( _("Hide banks details") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_banks, FALSE, FALSE, 0 );
    register_button_as_linked ( button_everything, button_banks );

    button_reports = gtk_check_button_new_with_label ( _("Hide reports names") );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_reports, FALSE, FALSE, 0 );
    register_button_as_linked ( button_everything, button_reports );

    button_reconcile = gtk_check_button_new_with_label (_("Hide reconciliation names and amounts"));
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), button_reconcile, FALSE, FALSE, 0 );
    register_button_as_linked ( button_everything, button_reconcile );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( button_everything ), TRUE );

    gtk_widget_show_all ( vbox );
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
    GtkWidget *vbox, *text_view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    gchar *text, *filename;
	const gchar *nom_fichier_comptes;

	vbox = gtk_box_new ( GTK_ORIENTATION_VERTICAL, MARGIN_BOX );

    text_view = gtk_text_view_new ();
    gtk_text_view_set_wrap_mode ( GTK_TEXT_VIEW ( text_view ), GTK_WRAP_WORD );
    gtk_text_view_set_editable ( GTK_TEXT_VIEW ( text_view ), FALSE );
    gtk_text_view_set_cursor_visible ( GTK_TEXT_VIEW ( text_view ), FALSE );
    gtk_text_view_set_left_margin ( GTK_TEXT_VIEW ( text_view ), 12 );
    gtk_text_view_set_right_margin ( GTK_TEXT_VIEW ( text_view ), 12 );

    buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( text_view ) );
    gtk_text_buffer_create_tag ( buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL );
    gtk_text_buffer_create_tag ( buffer, "x-large", "scale", PANGO_SCALE_X_LARGE, NULL );
    gtk_text_buffer_create_tag ( buffer, "indented", "left-margin", 24, NULL );

    gtk_text_buffer_get_iter_at_offset ( buffer, &iter, 1 );
    gtk_text_buffer_insert ( buffer, &iter, "\n", -1 );

	nom_fichier_comptes = grisbi_win_get_filename (NULL);
    if ( nom_fichier_comptes )
    {
        gchar *base_filename;
        gchar *complete_filename;

        base_filename = g_strdup ( nom_fichier_comptes );
        base_filename[strlen ( base_filename ) - 4] = 0;
        complete_filename = g_strconcat ( base_filename, "-obfuscated.gsb", NULL );
        filename = g_path_get_basename ( complete_filename );
        g_free ( complete_filename );
        g_free ( base_filename );
    }
    else
    {
        filename = g_strconcat ( "No_name-obfuscated.gsb", NULL );
    }

    text = g_strdup_printf ( _("Please press the 'Close' button to obfuscate your file\n\n"
                        "Obfuscated file will be named %s, in the same directory as original file.\n\n"
                        "Please check the bug is still there and send your file with the explanation to "
                        "make the bug at bugsreports@listes.grisbi.org or on the bugracker (Mantis) "
                        "at https://www.grisbi.org/bugsreports/\n\n"
                        "The account is saved in text, you may double check with a text editor if there "
                        "is no personal information anymore in this file."
                        "Grisbi will close immediately after saving the obfuscated file."),
                        filename );
    gtk_text_buffer_insert ( buffer, &iter, text, -1 );


    gtk_box_pack_start ( GTK_BOX ( vbox ), text_view, TRUE, TRUE, 0 );

    g_free ( text );
    g_free ( filename );

    gtk_widget_show_all ( vbox );

    return vbox;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
