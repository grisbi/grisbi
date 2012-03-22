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
#include <config.h>
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "traitement_variables.h"
#include "bet_data.h"
#include "bet_data_finance.h"
#include "bet_future.h"
#include "bet_graph.h"
#include "custom_list.h"
#include "fenetre_principale.h"
#include "grisbi_app.h"
#include "gsb_calendar.h"
#include "gsb_color.h"
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
#include "gsb_file_config.h"
#include "gsb_form_scheduler.h"
#include "gsb_form_widget.h"
#include "gsb_fyear.h"
#include "gsb_locale.h"
#include "gsb_real.h"
#include "gsb_regex.h"
#include "gsb_report.h"
#include "gsb_scheduler_list.h"
#include "gsb_select_icon.h"
#include "gsb_transactions_list.h"
#include "import.h"
#include "menu.h"
#include "navigation.h"
#include "transaction_model.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void initialise_format_date ( void );
static void initialise_number_separators ( void );
static void initialise_tab_affichage_ope ( void );
/*END_STATIC*/

gchar *titre_fichier = NULL;
gchar *adresse_commune = NULL;
gchar *adresse_secondaire = NULL;

 
/*START_EXTERN*/
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern GtkTreeModel *bank_list_model;
extern gint bet_array_col_width[BET_ARRAY_COLUMNS];
extern gint current_tree_view_width;
extern GtkWidget *detail_devise_compte;
extern gint display_one_line;
extern gint display_three_lines;
extern gint display_two_lines;
extern gint id_timeout;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern gint no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern GSList *orphan_child_transactions;
extern gint scheduler_col_width[SCHEDULER_COL_VISIBLE_COLUMNS];
extern gint scheduler_current_tree_view_width;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS];
extern gint transaction_col_align[CUSTOM_MODEL_VISIBLE_COLUMNS];
extern gint transaction_col_width[CUSTOM_MODEL_VISIBLE_COLUMNS];
/*END_EXTERN*/

/* the total of % of scheduled columns can be > 100 because all the columns are not showed at the same time */
static const gchar *scheduler_col_width_init = "10-12-36-12-12-12-12";
static const gchar *transaction_col_width_init = "10-12-36-6-12-12-12";

/**
 * initialisation of all the variables of grisbi
 * if some are not empty, free them before set it to NULL
 *
 * \param
 *
 * \return
 * */
void init_variables ( GrisbiWindowEtat *etat )
{
    gint bet_array_col_width_init[BET_ARRAY_COLUMNS] = {15, 40, 15, 15, 15 };
    gint transaction_col_align_init[CUSTOM_MODEL_VISIBLE_COLUMNS] = { 1, 1, 0, 1, 2, 2, 2 };
    gint i;
    GrisbiAppRun *run;

    devel_debug (NULL);

    run = grisbi_app_get_run ();

    /* init the format date */
    initialise_format_date ( );

    /* init the decimal point and the thousands separator. */
    initialise_number_separators ( );

    /* initialise l'ordre des pages du panneau de gauche */
    gsb_gui_navigation_init_pages_list ( );

    /* if ever there is still something from the previous list,
     * erase now */
    transaction_model_initialize();

    gsb_data_account_init_variables ();
    gsb_data_transaction_init_variables ();
    gsb_data_payee_init_variables ();
    gsb_data_category_init_variables ();
    gsb_data_budget_init_variables ();
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

    /* WARNING BUG */
/*     gsb_data_print_config_init ();  */

    /* no bank in memory for now */
    bank_list_model = NULL;

    mise_a_jour_liste_comptes_accueil = 0;
    mise_a_jour_liste_echeances_manuelles_accueil = 0;
    mise_a_jour_liste_echeances_auto_accueil = 0;
    mise_a_jour_soldes_minimaux = 0;
    mise_a_jour_fin_comptes_passifs = 0;

    orphan_child_transactions = NULL;
    
    affichage_echeances = SCHEDULER_PERIODICITY_ONCE_VIEW;
    affichage_echeances_perso_nb_libre = 0;
    affichage_echeances_perso_j_m_a = PERIODICITY_DAYS;

    no_devise_totaux_tiers = 1;
    no_devise_totaux_categ = 1;
    no_devise_totaux_ib = 1;

    /* initialization of titles and logo part */
    if ( titre_fichier && strlen ( titre_fichier ) )
        g_free ( titre_fichier );
    titre_fichier = g_strdup( _("My accounts") );

    etat->is_pixmaps_dir = TRUE;
    if ( etat->name_logo && strlen ( etat->name_logo ) )
        g_free ( etat->name_logo );
    etat->name_logo = NULL;
    etat->utilise_logo = 1;
    gsb_select_icon_init_logo_variables ();

    /* reconcile (etat) */
    run->reconcile_account_number = -1;
    g_free ( run->reconcile_final_balance );
    if ( run->reconcile_new_date )
        g_date_free ( run->reconcile_new_date );
    run->reconcile_final_balance = NULL;
    run->reconcile_new_date = NULL;

    adresse_commune = NULL;
    adresse_secondaire = NULL;

    current_tree_view_width = 0;
    scheduler_current_tree_view_width = 0;

    initialise_tab_affichage_ope();

    etat->valeur_echelle_recherche_date_import = 2;
    etat->get_fyear_by_value_date = FALSE;

    /* init default combofix values */
    etat->combofix_mixed_sort = FALSE;
    etat->combofix_max_item = 0;
    etat->combofix_case_sensitive = FALSE;
    etat->combofix_enter_select_completion = FALSE;
    etat->combofix_force_payee = FALSE;
    etat->combofix_force_category = FALSE;

    /* mis à NULL prévient un plantage aléatoire dans
     * gsb_currency_update_combobox_currency_list */
    detail_devise_compte = NULL;

    /* defaut value for width and align of columns */
    initialise_largeur_colonnes_tab_affichage_ope ( GSB_ACCOUNT_PAGE, transaction_col_width_init );
    initialise_largeur_colonnes_tab_affichage_ope ( GSB_SCHEDULER_PAGE, scheduler_col_width_init );
    for ( i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++ )
        transaction_col_align[i] = transaction_col_align_init[i];

    if ( etat->transaction_column_width && strlen ( etat->transaction_column_width ) )
    {
        g_free ( etat->transaction_column_width );
        etat->transaction_column_width = NULL;
    }
    if ( etat->scheduler_column_width && strlen ( etat->scheduler_column_width ) )
    {
        g_free ( etat->scheduler_column_width );
        etat->scheduler_column_width = NULL;
    }
    
    /* free the form */
    gsb_form_widget_free_list ();
    gsb_form_scheduler_free_list ();

    /* set colors to default */
    gsb_color_set_colors_to_default ( );

    /* divers */
    etat->add_archive_in_total_balance = TRUE;   /* add the archived transactions by default */
    etat->get_fyear_by_value_date = 0;           /* By default use transaction-date */
    etat->retient_affichage_par_compte = 0;
    memset ( etat->csv_skipped_lines, '\0', sizeof(gboolean) * CSV_MAX_TOP_LINES );

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
    etat->bet_deb_period = 1;
    /* defaut value for width of columns */
    for ( i = 0 ; i < BET_ARRAY_COLUMNS ; i++ )
        bet_array_col_width[i] = bet_array_col_width_init[i];

    bet_data_finance_data_simulator_init ( );

    bet_graph_set_configuration_variables ( NULL );
}

/**
 * Free allocations of grisbi variables
 *
 * */
void free_variables ( void )
{
    gsb_data_print_config_free ();
    gsb_gui_navigation_free_pages_list ( );
    gsb_regex_destroy ( );
    struct_free_bet_graph_prefs ( );
}


/*****************************************************************************************************/
/* si grise = 1 ; grise tous les menus qui doivent l'être quand aucun fichier n'est en mémoire */
/* si grise = 0 ; dégrise les même menus */
/*****************************************************************************************************/

void menus_sensitifs ( gboolean sensitif )
{
    gchar * items[] = {
        "/menubar/FileMenu/Save",
        "/menubar/FileMenu/SaveAs",
        "/menubar/FileMenu/DebugFile",
        "/menubar/FileMenu/Obfuscate",
        "/menubar/FileMenu/DebugMode",
        "/menubar/FileMenu/ExportFile",
        "/menubar/FileMenu/CreateArchive",
        "/menubar/FileMenu/ExportArchive",
        "/menubar/FileMenu/Close",
        "/menubar/EditMenu/NewTransaction",
        "/menubar/EditMenu/RemoveTransaction",
        "/menubar/EditMenu/TemplateTransaction",
        "/menubar/EditMenu/CloneTransaction",
        "/menubar/EditMenu/EditTransaction",
        "/menubar/EditMenu/ConvertToScheduled",
        "/menubar/EditMenu/MoveToAnotherAccount",
        "/menubar/EditMenu/Preferences",
        "/menubar/EditMenu/RemoveAccount",
        "/menubar/EditMenu/NewAccount",
        "/menubar/ViewMenu/ShowTransactionForm",
        "/menubar/ViewMenu/ShowGrid",
        "/menubar/ViewMenu/ShowReconciled",
        "/menubar/ViewMenu/ShowArchived",
        "/menubar/ViewMenu/ShowClosed",
        "/menubar/ViewMenu/ShowOneLine",
        "/menubar/ViewMenu/ShowTwoLines",
        "/menubar/ViewMenu/ShowThreeLines",
        "/menubar/ViewMenu/ShowFourLines",
        "/menubar/ViewMenu/InitwidthCol",
        NULL
    };
    gchar ** tmp = items;

    devel_debug_int (sensitif);

    while ( *tmp )
    {
        gsb_gui_sensitive_menu_item ( *tmp, sensitif );
        tmp++;
    }

    /* As this function may only be called when a new account is
     * created and the like, it is unlikely that we want to sensitive
     * transaction-related menus. */
    gsb_gui_sensitive_menu_item ( "/menubar/EditMenu/NewTransaction", FALSE );
    gsb_menu_transaction_operations_set_sensitive ( FALSE );
}

void menus_view_sensitifs ( gboolean sensitif )
{
    gchar * items[] = {
        "/menubar/ViewMenu/ShowTransactionForm",
        "/menubar/ViewMenu/ShowGrid",
        "/menubar/ViewMenu/ShowReconciled",
        "/menubar/ViewMenu/ShowArchived",
        "/menubar/ViewMenu/ShowClosed",
        "/menubar/ViewMenu/ShowOneLine",
        "/menubar/ViewMenu/ShowTwoLines",
        "/menubar/ViewMenu/ShowThreeLines",
        "/menubar/ViewMenu/ShowFourLines",
        "/menubar/ViewMenu/InitwidthCol",
        NULL
    };
    gchar **tmp = items;

    devel_debug_int (sensitif);

    while ( *tmp )
    {
        gsb_gui_sensitive_menu_item ( *tmp, sensitif );
        tmp++;
    }
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

    langue = g_getenv ( "LANG");

    if ( g_str_has_prefix ( langue, "en_" ) || g_str_has_prefix ( langue, "cs_" ) )
        gsb_date_set_format_date ( "%m/%d/%Y" );
    else
        gsb_date_set_format_date ( "%d/%m/%Y" );
}


/**
 * init the decimal point and the thousands separator.
 *
 * */
void initialise_number_separators ( void )
{
    struct lconv *conv;
    gchar *dec_point = NULL, *thousand_sep = NULL;

    gsb_locale_set_mon_decimal_point ( NULL );
    gsb_locale_set_mon_thousands_sep ( NULL );

    conv = localeconv();

    if ( conv->mon_decimal_point && strlen ( conv->mon_decimal_point ) )
    {
        dec_point = g_locale_to_utf8 ( conv->mon_decimal_point, -1, NULL, NULL, NULL );
        gsb_locale_set_mon_decimal_point ( dec_point );
        g_free ( dec_point );
    }
    else
        gsb_locale_set_mon_decimal_point ( "." );

    thousand_sep = g_locale_to_utf8 ( conv->mon_thousands_sep, -1, NULL, NULL, NULL );
    gsb_locale_set_mon_thousands_sep ( thousand_sep );
    g_free ( thousand_sep );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
