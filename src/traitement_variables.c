/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
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

#include "include.h"

/*START_INCLUDE*/
#include "traitement_variables.h"
#include "./gsb_currency.h"
#include "./gsb_data_account.h"
#include "./gsb_data_archive.h"
#include "./gsb_data_archive_store.h"
#include "./gsb_data_bank.h"
#include "./gsb_data_budget.h"
#include "./gsb_data_category.h"
#include "./gsb_data_currency.h"
#include "./gsb_data_currency_link.h"
#include "./gsb_data_fyear.h"
#include "./gsb_data_import_rule.h"
#include "./gsb_data_partial_balance.h"
#include "./gsb_data_payee.h"
#include "./gsb_data_payment.h"
#include "./gsb_data_print_config.h"
#include "./gsb_data_reconcile.h"
#include "./gsb_data_report_amout_comparison.h"
#include "./gsb_data_report.h"
#include "./gsb_data_report_text_comparison.h"
#include "./gsb_data_scheduled.h"
#include "./gsb_data_transaction.h"
#include "./gsb_form_scheduler.h"
#include "./gsb_form_widget.h"
#include "./gsb_fyear.h"
#include "./menu.h"
#include "./import.h"
#include "./gsb_report.h"
#include "./gsb_scheduler_list.h"
#include "./main.h"
#include "./transaction_model.h"
#include "./custom_list.h"
#include "./gsb_transactions_list.h"
#include "./include.h"
#include "./gsb_scheduler_list.h"
#include "./gsb_calendar.h"
#include "./erreur.h"
#include "./structures.h"
#include "./balance_estimate_data.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void initialise_tab_affichage_ope ( void );
/*END_STATIC*/

gchar *labels_titres_colonnes_liste_ope[] = {
    N_("Date"),
    N_("Value date"),
    N_("Payee"),
    N_("Budgetary lines"),
    N_("Debit"),
    N_("Credit"),
    N_("Balance"),
    N_("Amount"),
    N_("Method of payment"),
    N_("Reconciliation ref."),
    N_("Financial year"),
    N_("Category"),
    N_("C/R"),
    N_("Voucher"),
    N_("Notes"),
    N_("Bank references"),
    N_("Transaction number"),
    N_("Number"),
    NULL };



/** defaults colors in the transactions list */
GdkColor default_couleur_fond[2];
GdkColor default_couleur_jour;
GdkColor default_couleur_grise;
GdkColor default_archive_background_color;
GdkColor default_couleur_selection;
GdkColor default_split_background;
GdkColor default_text_color[2];
GdkColor default_calendar_entry_color;

/** colors in the transactions list */
GdkColor couleur_fond[2];
GdkColor couleur_jour;
GdkColor couleur_grise;
GdkColor archive_background_color;
GdkColor couleur_selection;
GdkColor split_background;
GdkColor text_color[2];
GdkColor calendar_entry_color;

/* colors of the amounts in the first page */
GdkColor couleur_solde_alarme_verte_normal;
GdkColor couleur_solde_alarme_verte_prelight;
GdkColor couleur_solde_alarme_orange_normal;
GdkColor couleur_solde_alarme_orange_prelight;
GdkColor couleur_solde_alarme_rouge_normal;
GdkColor couleur_solde_alarme_rouge_prelight;
GdkColor couleur_nom_compte_normal;
GdkColor couleur_nom_compte_prelight;

GdkColor couleur_bleue;
GdkColor couleur_jaune;
GdkColor couleur_bet_division;
GdkColor default_couleur_bet_division;
 
GSList *liste_labels_titres_colonnes_liste_ope = NULL;

gchar *nom_fichier_comptes = NULL;

gchar *titre_fichier = NULL;
gchar *adresse_commune = NULL;
gchar *adresse_secondaire = NULL;

 
/*START_EXTERN*/
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern GtkTreeModel *bank_list_model;
extern GtkTreeStore *categ_tree_model;
extern gint current_tree_view_width;
extern GtkWidget *detail_devise_compte;
extern gint display_one_line;
extern gint display_three_lines;
extern gint display_two_lines;
extern gint id_timeout;
extern gchar *initial_file_title;
extern gchar *initial_holder_title;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkWidget * navigation_tree_view;
extern gint no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern GtkWidget *notebook_general;
extern GSList *orphan_child_transactions;
extern gint scheduler_col_width[SCHEDULER_COL_VISIBLE_COLUMNS];
extern gint scheduler_current_tree_view_width;
extern GtkWidget *solde_label;
extern GtkWidget *solde_label_pointe;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][CUSTOM_MODEL_VISIBLE_COLUMNS];
extern gint transaction_col_width[CUSTOM_MODEL_N_VISIBLES_COLUMN];
extern gint valeur_echelle_recherche_date_import;
/*END_EXTERN*/



/**
 * set or unset the modified flag
 * and sensitive or not the menu to save the file
 *
 * \param modif TRUE to set the modified flag, FALSE to unset
 *
 * \return
 * */
void modification_fichier ( gboolean modif )
{
    devel_debug_int (modif);

    /* If no file is loaded, do not change menu items. */
    if ( ! gsb_data_account_get_accounts_amount () )
    {
        return;
    }

    if ( modif )
    {
    if ( ! etat.modification_fichier )
    {
        etat.modification_fichier = time ( NULL );
        gsb_gui_sensitive_menu_item ( "FileMenu", "Save", NULL, TRUE );
    }
    }
    else
    {
        etat.modification_fichier = 0;
        gsb_gui_sensitive_menu_item ( "FileMenu", "Save", NULL, FALSE );
    }
}



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
    /* the total of % of scheduled columns can be > 100 because all the columns are not showed at the same time */
    gint scheduler_col_width_init[SCHEDULER_COL_VISIBLE_COLUMNS] = {10, 12, 36, 12, 12, 12, 12 };
    gint transaction_col_width_init[CUSTOM_MODEL_VISIBLE_COLUMNS] = {10, 12, 36, 6, 12, 12, 12 };
    gint i;
    
/* xxx on devrait séparer ça en 2 : les variables liées au fichier de compte, qui doivent être remises  à 0,
 * et les variables liées à grisbi (ex sauvegarde auto...) qui doivent rester */
    devel_debug (NULL);

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

    gsb_data_print_config_init ();

    /* no bank in memory for now */
    bank_list_model = NULL;

    mise_a_jour_liste_comptes_accueil = 0;
    mise_a_jour_liste_echeances_manuelles_accueil = 0;
    mise_a_jour_liste_echeances_auto_accueil = 0;
    mise_a_jour_soldes_minimaux = 0;
    mise_a_jour_fin_comptes_passifs = 0;

    orphan_child_transactions = NULL;
    
    /* the main notebook is set to NULL,
     * important because it's the checked variable in a new file
     * to know if the widgets are created or not */
    notebook_general = NULL;

    if ( nom_fichier_comptes )
        g_free ( nom_fichier_comptes );
    nom_fichier_comptes = NULL;

    solde_label = NULL;
    solde_label_pointe = NULL;

    affichage_echeances = SCHEDULER_PERIODICITY_ONCE_VIEW;
    affichage_echeances_perso_nb_libre = 0;
    affichage_echeances_perso_j_m_a = PERIODICITY_DAYS;


    no_devise_totaux_tiers = 1;
    no_devise_totaux_categ = 1;
    no_devise_totaux_ib = 1;

    /* initialization of titles */
    titre_fichier = g_strdup( _("My accounts") );
    if ( initial_file_title && strlen ( initial_file_title ) )
        g_free ( initial_file_title );
    initial_file_title = NULL;

    if ( initial_holder_title && strlen ( initial_holder_title ) )
        g_free ( initial_holder_title );
    initial_holder_title = NULL;

    adresse_commune = NULL;
    adresse_secondaire = NULL;

    etat.largeur_auto_colonnes = 1;
    current_tree_view_width = 0;
    scheduler_current_tree_view_width = 0;

    initialise_tab_affichage_ope();

    valeur_echelle_recherche_date_import = 2;
    etat.get_fyear_by_value_date = FALSE;

    /* 	on initialise la liste des labels des titres de colonnes */
    if ( !liste_labels_titres_colonnes_liste_ope )
    {
    i=0;
    while ( labels_titres_colonnes_liste_ope[i] )
    {
        liste_labels_titres_colonnes_liste_ope = g_slist_append (
                                    liste_labels_titres_colonnes_liste_ope,
                                    labels_titres_colonnes_liste_ope[i] );
        i++;
    }
    }

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

    /* defaut value for width of columns */
    for ( i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++ )
    transaction_col_width[i] = transaction_col_width_init[i];
     for ( i = 0 ; i < SCHEDULER_COL_VISIBLE_COLUMNS ; i++ )
    scheduler_col_width[i] = scheduler_col_width_init[i];
    

    navigation_tree_view = NULL;

    /* free the form */
    gsb_form_widget_free_list ();
    gsb_form_scheduler_free_list ();

    /* set colors to default */
    couleur_fond[0] = default_couleur_fond[0];
    couleur_fond[1] = default_couleur_fond[1];
    couleur_jour = default_couleur_jour;
    couleur_grise = default_couleur_grise;
    archive_background_color = default_archive_background_color;
    couleur_selection = default_couleur_selection;
    split_background = default_split_background;
    text_color[0] = default_text_color[0];
    text_color[1] = default_text_color[1];
    calendar_entry_color = default_calendar_entry_color;

    etat.add_archive_in_total_balance = TRUE;   /* add the archived transactions by default */

    /* remove the timeout if necessary */
    if (id_timeout)
    {
    g_source_remove (id_timeout);
    id_timeout = 0;
    }

    /* initialise la liste des catégories */
    categ_tree_model = NULL;

    /* initializes the variables for the estimate balance module */
#ifdef ENABLE_BALANCE_ESTIMATE
    /* création de la liste des données à utiliser dans le tableau de résultats */
    bet_data_init_variables ( );
#endif /* ENABLE_BALANCE_ESTIMATE */
    etat.bet_deb_period = 1;
}


/**
 * initialize the colors used in grisbi 
 *
 * \param
 *
 * \return
 * */
void initialisation_couleurs_listes ( void )
{
    devel_debug (NULL);

    /* colors of the background */
    default_couleur_fond[0].red = BG_COLOR_1_RED;
    default_couleur_fond[0].green = BG_COLOR_1_GREEN;
    default_couleur_fond[0].blue = BG_COLOR_1_BLUE;

    default_couleur_fond[1].red = BG_COLOR_2_RED;
    default_couleur_fond[1].green = BG_COLOR_2_GREEN;
    default_couleur_fond[1].blue = BG_COLOR_2_BLUE;

    /* color of today */
    default_couleur_jour.red = BG_COLOR_TODAY_RED;
    default_couleur_jour.green = BG_COLOR_TODAY_GREEN;
    default_couleur_jour.blue = BG_COLOR_TODAY_BLUE;

    /* colors of the text */
    default_text_color[0].red = TEXT_COLOR_1_RED;
    default_text_color[0].green = TEXT_COLOR_1_GREEN;
    default_text_color[0].blue = TEXT_COLOR_1_BLUE;

    default_text_color[1].red = TEXT_COLOR_2_RED;
    default_text_color[1].green = TEXT_COLOR_2_GREEN;
    default_text_color[1].blue = TEXT_COLOR_2_BLUE;

    /* selection color */
    default_couleur_selection.red= SELECTION_COLOR_RED;
    default_couleur_selection.green= SELECTION_COLOR_GREEN ;
    default_couleur_selection.blue= SELECTION_COLOR_BLUE;

    /* color of the non selectable transactions on scheduler */
    default_couleur_grise.red = UNSENSITIVE_SCHEDULED_COLOR_RED;
    default_couleur_grise.green = UNSENSITIVE_SCHEDULED_COLOR_GREEN;
    default_couleur_grise.blue = UNSENSITIVE_SCHEDULED_COLOR_BLUE;
    default_couleur_grise.pixel = 0;

    /* set the archive background color */
    default_archive_background_color.red = COLOR_ARCHIVE_BG_RED;
    default_archive_background_color.green = COLOR_ARCHIVE_BG_GREEN;
    default_archive_background_color.blue = COLOR_ARCHIVE_BG_BLUE;
    default_archive_background_color.pixel = 0;

    /* color for split children */
    default_split_background.red = BREAKDOWN_BACKGROUND_COLOR_RED;
    default_split_background.green = BREAKDOWN_BACKGROUND_COLOR_GREEN;
    default_split_background.blue = BREAKDOWN_BACKGROUND_COLOR_BLUE;
    default_split_background.pixel = 0;

    /* color for wrong entry in calendar entry */
    default_calendar_entry_color.red = CALENDAR_ENTRY_COLOR_RED;
    default_calendar_entry_color.green = CALENDAR_ENTRY_COLOR_GREEN;
    default_calendar_entry_color.blue = CALENDAR_ENTRY_COLOR_BLUE;
    default_calendar_entry_color.pixel = 0;

    /* Initialisation des couleurs des différents labels */
    /* Pourra être intégré à la configuration générale */
    couleur_solde_alarme_verte_normal.red =     0.00 * 65535 ;
    couleur_solde_alarme_verte_normal.green =   0.50 * 65535 ;
    couleur_solde_alarme_verte_normal.blue =    0.00 * 65535 ;
    couleur_solde_alarme_verte_normal.pixel = 1;

    couleur_solde_alarme_verte_prelight.red =   0.00 * 65535 ;
    couleur_solde_alarme_verte_prelight.green = 0.90 * 65535 ;
    couleur_solde_alarme_verte_prelight.blue =  0.00 * 65535 ;
    couleur_solde_alarme_verte_prelight.pixel = 1;


    couleur_solde_alarme_orange_normal.red =     0.90 * 65535 ;
    couleur_solde_alarme_orange_normal.green =   0.60 * 65535 ;
    couleur_solde_alarme_orange_normal.blue =    0.00 * 65535 ;
    couleur_solde_alarme_orange_normal.pixel = 1;

    couleur_solde_alarme_orange_prelight.red =   1.00 * 65535 ;
    couleur_solde_alarme_orange_prelight.green = 0.80 * 65535 ;
    couleur_solde_alarme_orange_prelight.blue =  0.00 * 65535 ;
    couleur_solde_alarme_orange_prelight.pixel = 1;


    couleur_solde_alarme_rouge_normal.red =     0.60 * 65535 ;
    couleur_solde_alarme_rouge_normal.green =   0.00 * 65535 ;
    couleur_solde_alarme_rouge_normal.blue =    0.00 * 65535 ;
    couleur_solde_alarme_rouge_normal.pixel = 1;

    couleur_solde_alarme_rouge_prelight.red =   1.00 * 65535 ;
    couleur_solde_alarme_rouge_prelight.green = 0.00 * 65535 ;
    couleur_solde_alarme_rouge_prelight.blue =  0.00 * 65535 ;
    couleur_solde_alarme_rouge_prelight.pixel = 1;


    couleur_nom_compte_normal.red =     0.00 * 65535 ;
    couleur_nom_compte_normal.green =   0.00 * 65535 ;
    couleur_nom_compte_normal.blue =    0.00 * 65535 ;
    couleur_nom_compte_normal.pixel = 1;

    couleur_nom_compte_prelight.red =   0.61 * 65535 ;
    couleur_nom_compte_prelight.green = 0.61 * 65535 ;
    couleur_nom_compte_prelight.blue =  0.61 * 65535 ;
    couleur_nom_compte_prelight.pixel = 1;

    couleur_bleue.red = 500;
    couleur_bleue.green = 500;
    couleur_bleue.blue = 65535;
    couleur_bleue.pixel = 1;


    couleur_jaune.red = 50535;
    couleur_jaune.green = 65535;
    couleur_jaune.blue = 0;
    couleur_jaune.pixel = 1;

    /* colors of the background bet divisions */
    couleur_bet_division.red =64550;
    couleur_bet_division.green =65535;
    couleur_bet_division.blue = 33466;

    default_couleur_bet_division.red =64550;
    default_couleur_bet_division.green =65535;
    default_couleur_bet_division.blue = 33466;

    
}
/*****************************************************************************************************/

/*****************************************************************************************************/
/* si grise = 1 ; grise tous les menus qui doivent l'être quand aucun fichier n'est en mémoire */
/* si grise = 0 ; dégrise les même menus */
/*****************************************************************************************************/

void menus_sensitifs ( gboolean sensitif )
{
    gchar * items[] = {
    menu_name ( "FileMenu",     "Save",                 NULL ),
    menu_name ( "FileMenu",     "SaveAs",               NULL ),
    menu_name ( "FileMenu",     "DebugFile",            NULL ),
    menu_name ( "FileMenu",     "Obfuscate",            NULL ),
    menu_name ( "FileMenu",     "DebugMode",            NULL ),
    menu_name ( "FileMenu",     "ExportFile",           NULL ),
    menu_name ( "FileMenu",     "CreateArchive",        NULL ),
    menu_name ( "FileMenu",     "ExportArchive",        NULL ),
    menu_name ( "FileMenu",     "Close",                NULL ),
    menu_name ( "EditMenu",     "NewTransaction",       NULL ),
    menu_name ( "EditMenu",     "RemoveTransaction",    NULL ),
    menu_name ( "EditMenu",     "CloneTransaction",     NULL ),
    menu_name ( "EditMenu",     "EditTransaction",      NULL ),
    menu_name ( "EditMenu",     "ConvertToScheduled",   NULL ),
    menu_name ( "EditMenu",     "MoveToAnotherAccount", NULL ),
    menu_name ( "EditMenu",     "Preferences",          NULL ),
    menu_name ( "EditMenu",     "RemoveAccount",        NULL ),
    menu_name ( "EditMenu",     "NewAccount",           NULL ),
    menu_name ( "ViewMenu",     "ShowTransactionForm",  NULL ),
    menu_name ( "ViewMenu",     "ShowGrid",             NULL ),
    menu_name ( "ViewMenu",     "ShowReconciled",       NULL ),
    menu_name ( "ViewMenu",     "ShowClosed",           NULL ),
    menu_name ( "ViewMenu",     "ShowOneLine",          NULL ),
    menu_name ( "ViewMenu",     "ShowTwoLines",         NULL ),
    menu_name ( "ViewMenu",     "ShowThreeLines",       NULL ),
    menu_name ( "ViewMenu",     "ShowFourLines",        NULL ),
    NULL,
    };
    gchar ** tmp = items;

    devel_debug_int (sensitif);

    while ( *tmp )
    {
    gsb_gui_sensitive_menu_item_from_string ( *tmp, sensitif );
    tmp++;
    }

    /* As this function may only be called when a new account is
     * created and the like, it is unlikely that we want to sensitive
     * transaction-related menus. */
    gsb_gui_sensitive_menu_item ( "EditMenu", "NewTransaction", NULL, FALSE );
    gsb_menu_transaction_operations_set_sensitive ( FALSE );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
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

    /* by default, the display of lines is 1, 1-2, 1-2-4 */
    display_one_line = 0;
    display_two_lines = 0;
    display_three_lines = 1;
}
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
