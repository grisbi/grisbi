/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*                      2009-2011 Pierre Biava (grisbi@pierre.biava.name)     */
/*          http://www.grisbi.org                                             */
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
 * \file traitement_variables.c
 * works with global variables of grisbi (initialisation...)
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "traitement_variables.h"
#include "bet_data.h"
#include "bet_data_finance.h"
#include "bet_future.h"
#include "bet_graph.h"
#include "categories_onglet.h"
#include "custom_list.h"
#include "fenetre_principale.h"
#include "grisbi_win.h"
#include "gsb_calendar.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_archive.h"
#include "gsb_data_archive_store.h"
#include "gsb_data_bank.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_currency_link.h"
#include "gsb_data_fyear.h"
#include "gsb_data_import_rule.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_payee.h"
#include "gsb_data_payment.h"
#include "gsb_data_print_config.h"
#include "gsb_data_reconcile.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report.h"
#include "gsb_data_report_text_comparison.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_form_scheduler.h"
#include "gsb_form_widget.h"
#include "gsb_fyear.h"
#include "gsb_locale.h"
#include "gsb_real.h"
#include "gsb_regex.h"
#include "gsb_report.h"
#include "gsb_rgba.h"
#include "gsb_scheduler_list.h"
#include "gsb_select_icon.h"
#include "gsb_transactions_list.h"
#include "import.h"
#include "imputation_budgetaire.h"
#include "menu.h"
#include "navigation.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "transaction_model.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void initialise_format_date ( void );
static void initialise_tab_affichage_ope ( void );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkTreeModel *bank_list_model;
extern gint bet_array_col_width[BET_ARRAY_COLUMNS];
extern gint current_tree_view_width;
extern GtkWidget *detail_devise_compte;
extern gint display_one_line;
extern gint display_three_lines;
extern gint display_two_lines;
extern gint id_timeout;
extern GSList *orphan_child_transactions;
extern gint scheduler_col_width[SCHEDULER_COL_VISIBLE_COLUMNS];
extern gint scheduler_current_tree_view_width;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS];
extern gint transaction_col_align[CUSTOM_MODEL_VISIBLE_COLUMNS];
extern gint transaction_col_width[CUSTOM_MODEL_VISIBLE_COLUMNS];
/*END_EXTERN*/

/* the total of % of scheduled columns can be > 100 because all the columns are not showed at the same time */
static const gchar *scheduler_col_width_init = "10-12-36-12-12-12-12";
static const gchar *transaction_col_width_init = "10-12-30-12-12-12-12";

/**
 * initialisation of all the variables of grisbi
 * if some are not empty, free them before set it to NULL
 *
 * \param
 *
 * \return
 * */
void init_variables ( void )
{
    gint bet_array_col_width_init[BET_ARRAY_COLUMNS] = {15, 40, 15, 15, 15 };
    gint transaction_col_align_init[CUSTOM_MODEL_VISIBLE_COLUMNS] = { 1, 1, 0, 1, 2, 2, 2 };
    gint i;

    devel_debug (NULL);

    /* init the new crypted file */
    run.new_crypted_file = FALSE;

	/* init the format date */
    initialise_format_date ( );

    /* initialise l'ordre des pages du panneau de gauche */
    gsb_gui_navigation_init_pages_list ( );

    /* if ever there is still something from the previous list,
     * erase now */
    transaction_model_set_model ( NULL );

    gsb_data_account_init_variables ();
    gsb_data_transaction_init_variables ();
    gsb_data_payee_init_variables ();
    payees_init_variables_list ();
    gsb_data_category_init_variables ();
    categories_init_variables_list ();
    gsb_data_budget_init_variables ();
    budgetary_lines_init_variables_list ();
    gsb_data_report_init_variables ();
    gsb_data_report_amount_comparison_init_variables ();
    gsb_data_report_text_comparison_init_variables ();
    gsb_data_scheduled_init_variables ();
    gsb_scheduler_list_init_variables ();
    gsb_data_currency_init_variables ();
    gsb_data_currency_link_init_variables ();
    gsb_data_fyear_init_variables ();
    gsb_data_bank_init_variables ();
    gsb_data_reconcile_init_variables ();
    gsb_data_payment_init_variables ();
    gsb_data_archive_init_variables ();
    gsb_data_archive_store_init_variables ();
    gsb_data_import_rule_init_variables ();
    gsb_import_associations_init_variables ( );
    gsb_data_partial_balance_init_variables ( );

    gsb_currency_init_variables ();
    gsb_fyear_init_variables ();
    gsb_report_init_variables ();
    gsb_regex_init_variables ();

    gsb_data_print_config_init ();

    /* no bank in memory for now */
    bank_list_model = NULL;

    run.mise_a_jour_liste_comptes_accueil = FALSE;
    run.mise_a_jour_liste_echeances_manuelles_accueil = FALSE;
    run.mise_a_jour_liste_echeances_auto_accueil = FALSE;
    run.mise_a_jour_soldes_minimaux = FALSE;
    run.mise_a_jour_fin_comptes_passifs = FALSE;

    orphan_child_transactions = NULL;

    etat.affichage_echeances = SCHEDULER_PERIODICITY_ONCE_VIEW;
    etat.affichage_echeances_perso_nb_libre = 0;
    etat.affichage_echeances_perso_j_m_a = PERIODICITY_DAYS;

    if ( etat.name_logo && strlen ( etat.name_logo ) )
        g_free ( etat.name_logo );
    etat.name_logo = NULL;
    etat.utilise_logo = 1;
    gsb_select_icon_init_logo_variables ();

    etat.retient_affichage_par_compte = 0;

    /* reconcile (etat) */
    run.reconcile_account_number = -1;
    g_free ( run.reconcile_final_balance );
    if ( run.reconcile_new_date )
        g_date_free ( run.reconcile_new_date );
    run.reconcile_final_balance = NULL;
    run.reconcile_new_date = NULL;

    current_tree_view_width = 0;
    scheduler_current_tree_view_width = 0;

    initialise_tab_affichage_ope();

    etat.import_files_nb_days = 2;
    etat.get_fyear_by_value_date = FALSE;

    /* init default combofix values */
    etat.combofix_mixed_sort = FALSE;
    etat.combofix_max_item = 0;
    etat.combofix_case_sensitive = FALSE;
    etat.combofix_enter_select_completion = FALSE;
    etat.combofix_force_payee = FALSE;
    etat.combofix_force_category = FALSE;

    /* mis à NULL prévient un plantage aléatoire dans
     * gsb_currency_update_combobox_currency_list */
    detail_devise_compte = NULL;

    /* the main notebook is set to NULL,
     * important because it's the checked variable in a new file
     * to know if the widgets are created or not */
    grisbi_win_free_general_notebook ();

    /* defaut value for width and align of columns */
    initialise_largeur_colonnes_tab_affichage_ope ( GSB_ACCOUNT_PAGE, transaction_col_width_init );
    initialise_largeur_colonnes_tab_affichage_ope ( GSB_SCHEDULER_PAGE, scheduler_col_width_init );
    for ( i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++ )
        transaction_col_align[i] = transaction_col_align_init[i];

    if ( etat.transaction_column_width && strlen ( etat.transaction_column_width ) )
    {
        g_free ( etat.transaction_column_width );
        etat.transaction_column_width = NULL;
    }
    if ( etat.scheduler_column_width && strlen ( etat.scheduler_column_width ) )
    {
        g_free ( etat.scheduler_column_width );
        etat.scheduler_column_width = NULL;
    }

    gsb_gui_navigation_init_tree_view ( );

    /* free the form */
    gsb_form_widget_free_list ();
    gsb_form_scheduler_free_list ();

    /* set colors to default */
    gsb_rgba_set_colors_to_default ();

    /* divers */
    etat.add_archive_in_total_balance = TRUE;   /* add the archived transactions by default */
    etat.get_fyear_by_value_date = 0;           /* By default use transaction-date */

    /* remove the timeout if necessary */
    if (id_timeout)
    {
    g_source_remove (id_timeout);
    id_timeout = 0;
    }

    /* initializes the variables for the estimate balance module */
    /* création de la liste des données à utiliser dans le tableau de résultats */
    bet_data_init_variables ( );
    /* initialisation des boites de dialogue */
    bet_future_initialise_dialog ( );
    etat.bet_deb_period = 1;
    /* defaut value for width of columns */
    for ( i = 0 ; i < BET_ARRAY_COLUMNS ; i++ )
        bet_array_col_width[i] = bet_array_col_width_init[i];

    bet_data_finance_data_simulator_init ( );

#ifdef HAVE_GOFFICE
    bet_graph_set_configuration_variables ( NULL );
#endif /* HAVE_GOFFICE */

}

/**
 * Free allocations of grisbi variables
 *
 * */
void free_variables (void)
{
	gsb_data_print_config_free ();
	gsb_data_import_rule_free_list ();
	gsb_gui_navigation_free_pages_list ();
	gsb_import_associations_free_liste ();
    gsb_regex_destroy ();
    bet_data_free_variables ();
	if (etat.csv_separator)
		g_free (etat.csv_separator);

#ifdef HAVE_GOFFICE
    struct_free_bet_graph_prefs ();
#endif /* HAVE_GOFFICE */
}


/**
 * initialise la largeur des colonnes du tableau d'affichage des opérations.
 * ou des opérations planifiées.
 *
 * */
void initialise_largeur_colonnes_tab_affichage_ope ( gint type_operation, const gchar *description )
{
    gchar **pointeur_char;
    gint j;

    if ( description == NULL )
    {
        if ( type_operation == GSB_ACCOUNT_PAGE )
            description = transaction_col_width_init;
        else if ( type_operation == GSB_SCHEDULER_PAGE )
            description = scheduler_col_width_init;
    }

    /* the transactions columns are xx-xx-xx-xx and we want to set in transaction_col_width[1-2-3...] */
    pointeur_char = g_strsplit ( description, "-", 0 );

    if ( type_operation == GSB_ACCOUNT_PAGE )
    {
        for ( j = 0 ; j < CUSTOM_MODEL_VISIBLE_COLUMNS ; j++ )
            transaction_col_width[j] = utils_str_atoi ( pointeur_char[j] );
    }
    else if ( type_operation == GSB_SCHEDULER_PAGE )
    {
        for ( j = 0 ; j < SCHEDULER_COL_VISIBLE_COLUMNS ; j++ )
            scheduler_col_width[j] = utils_str_atoi ( pointeur_char[j] );
    }

        g_strfreev ( pointeur_char );
}


/**
 * initialise le contenu du tableau d'affichage des opérations.
 *
 * */
void initialise_tab_affichage_ope ( void )
{
    gint tab[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS] = {
    { ELEMENT_CHQ, ELEMENT_DATE, ELEMENT_PARTY, ELEMENT_MARK, ELEMENT_DEBIT, ELEMENT_CREDIT, ELEMENT_BALANCE },
    {0, 0, ELEMENT_CATEGORY, 0, ELEMENT_PAYMENT_TYPE, ELEMENT_AMOUNT, 0 },
    {0, 0, ELEMENT_NOTES, 0, 0, 0, 0 },
    {0, 0, 0, 0, 0, 0, 0 }
    };
    gint i, j;

    devel_debug (NULL);

    for ( i = 0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
    for ( j = 0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++ )
        tab_affichage_ope[i][j] = tab[i][j];

    /* by default, the display of lines is 1, 1-2, 1-2-3 */
    display_one_line = 0;
    display_two_lines = 0;
    display_three_lines = 0;
}


/**
 * init the format of date.
 *
 * */
void initialise_format_date ( void )
{
    const gchar *langue;

    gsb_date_set_format_date ( NULL );

    langue = g_getenv ("LANG");
	if (langue)
	{
		if ( g_str_has_prefix ( langue, "en_" ) || g_str_has_prefix ( langue, "cs_" ) )
			gsb_date_set_format_date ( "%m/%d/%Y" );
		else if ( g_str_has_prefix ( langue, "de_" ) )
			gsb_date_set_format_date ( "%d.%m.%Y" );
		else
			gsb_date_set_format_date ( "%d/%m/%Y" );
	}
    else
	{
		gsb_date_set_format_date ( "%d/%m/%Y" );
	}
}


/**
 *
 *
 * \param
 *
 * \return
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
