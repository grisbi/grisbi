/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2008 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "./gsb_report.h"
#include "./gsb_scheduler_list.h"
#include "./main.h"
#include "./transaction_model.h"
#include "./custom_list.h"
#include "./gsb_transactions_list.h"
#include "./gsb_scheduler_list.h"
#include "./include.h"
#include "./gsb_calendar.h"
#include "./erreur.h"
#include "./structures.h"
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



GdkColor couleur_fond[2];
GdkColor couleur_grise;
GdkColor archive_background_color;
GdkColor couleur_selection;
GdkColor split_background;


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

/* ajouté pour la gestion des boutons afficher/masquer les opérations rapprochées */
GtkIconFactory *gsb_factory;

GSList *liste_labels_titres_colonnes_liste_ope = NULL;

gchar *nom_fichier_comptes = NULL;

gchar *titre_fichier = NULL;
gchar *adresse_commune = NULL;
gchar *adresse_secondaire = NULL;



/*START_EXTERN*/
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern GtkTreeModel *bank_list_model ;
extern gchar *chemin_logo ;
extern gint current_tree_view_width ;
extern gint ligne_affichage_une_ligne;
extern GSList *lignes_affichage_deux_lignes;
extern GSList *lignes_affichage_trois_lignes;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern GtkWidget * navigation_tree_view ;
extern gint no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern GtkWidget *notebook_general ;
extern GSList *orphan_child_transactions ;
extern gint scheduler_col_width[NB_COLS_SCHEDULER];
extern GtkWidget *solde_label ;
extern GtkWidget *solde_label_pointe ;
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
    gint scheduler_col_width_init[NB_COLS_SCHEDULER] = {119, 121, 352, 129, 147, 0, 116};
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

    if ( chemin_logo )
        g_free ( chemin_logo);
    chemin_logo = NULL;

    affichage_echeances = SCHEDULER_PERIODICITY_ONCE_VIEW;
    affichage_echeances_perso_nb_libre = 0;
    affichage_echeances_perso_j_m_a = PERIODICITY_DAYS;


    no_devise_totaux_tiers = 1;
    no_devise_totaux_categ = 1;
    no_devise_totaux_ib = 1;

    titre_fichier = g_strdup(_("My accounts"));
    adresse_commune = NULL;
    adresse_secondaire = NULL;

    etat.largeur_auto_colonnes = 1;
    current_tree_view_width = 0;

    initialise_tab_affichage_ope();

    valeur_echelle_recherche_date_import = 2;


    /* 	on initialise la liste des labels des titres de colonnes */
    if ( !liste_labels_titres_colonnes_liste_ope )
    {
	i=0;
	while ( labels_titres_colonnes_liste_ope[i] )
	{
	    liste_labels_titres_colonnes_liste_ope = g_slist_append ( liste_labels_titres_colonnes_liste_ope,
								      labels_titres_colonnes_liste_ope[i] );
	    i++;
	}
    }

    /* init default combofix values */
    etat.combofix_mixed_sort = FALSE;
    etat.combofix_max_item = FALSE;
    etat.combofix_case_sensitive = FALSE;
    etat.combofix_enter_select_completion = FALSE;
    etat.combofix_force_payee = FALSE;
    etat.combofix_force_category = FALSE;
    
    /* archive stuff */
    etat.check_for_archival = TRUE;
    etat.max_non_archived_transactions_for_check = 2000;

    /* defaut value for width of columns */
    for ( i = 0 ; i < CUSTOM_MODEL_VISIBLE_COLUMNS ; i++ )
	transaction_col_width[i] = transaction_col_width_init[i];
     for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
	scheduler_col_width[i] = scheduler_col_width_init[i];
    

    navigation_tree_view = NULL;

    /* free the form */
    gsb_form_widget_free_list ();
    gsb_form_scheduler_free_list ();
}
/*****************************************************************************************************/





/*****************************************************************************************************/
void initialisation_couleurs_listes ( void )
{
    GdkColor couleur_rouge;

    devel_debug ("initialisation_couleurs_listes");

    /* Initialisation des couleurs de la clist */

    couleur_fond[0].red = COULEUR1_RED ;
    couleur_fond[0].green = COULEUR1_GREEN;
    couleur_fond[0].blue = COULEUR1_BLUE;

    couleur_fond[1].red = COULEUR2_RED;
    couleur_fond[1].green = COULEUR2_GREEN;
    couleur_fond[1].blue = COULEUR2_BLUE;

    couleur_rouge.red = COULEUR_ROUGE_RED;
    couleur_rouge.green = COULEUR_ROUGE_GREEN;
    couleur_rouge.blue = COULEUR_ROUGE_BLUE;
    couleur_rouge.pixel = 0;

    /*     couleur grise de l'échéancier */
    couleur_grise.red = COULEUR_GRISE_RED;
    couleur_grise.green = COULEUR_GRISE_GREEN;
    couleur_grise.blue = COULEUR_GRISE_BLUE;
    couleur_grise.pixel = 0;

    /* set the archive background color */
    archive_background_color.red = COLOR_ARCHIVE_BG_RED;
    archive_background_color.green = COLOR_ARCHIVE_BG_GREEN;
    archive_background_color.blue = COLOR_ARCHIVE_BG_BLUE;
    archive_background_color.pixel = 0;

    /* color for split children */

    split_background.red = BREAKDOWN_BACKGROUND_COLOR_RED;
    split_background.green = BREAKDOWN_BACKGROUND_COLOR_GREEN;
    split_background.blue = BREAKDOWN_BACKGROUND_COLOR_BLUE;
    split_background.pixel = 0;

    /* initialise la couleur de la sélection */

    couleur_selection.red= COULEUR_SELECTION_RED;
    couleur_selection.green= COULEUR_SELECTION_GREEN ;
    couleur_selection.blue= COULEUR_SELECTION_BLUE;

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


    couleur_jaune.red =40000;
    couleur_jaune.green =40000;
    couleur_jaune.blue = 0;
    couleur_jaune.pixel = 1;
}
/*****************************************************************************************************/

/*****************************************************************************************************/
/* si grise = 1 ; grise tous les menus qui doivent l'être quand aucun fichier n'est en mémoire */
/* si grise = 0 ; dégrise les même menus */
/*****************************************************************************************************/

void menus_sensitifs ( gboolean sensitif )
{
    gchar * items[] = {
	menu_name ( "FileMenu",		"Save",			NULL ),
	menu_name ( "FileMenu",		"SaveAs",		NULL ),
	menu_name ( "FileMenu",		"DebugFile",		NULL ),
	menu_name ( "FileMenu",		"Obfuscate",		NULL ),
	menu_name ( "FileMenu",		"ExportFile",		NULL ),
	menu_name ( "FileMenu",		"CreateArchive",	NULL ),
	menu_name ( "FileMenu",		"ExportArchive",	NULL ),
	menu_name ( "FileMenu",		"Close",		NULL ),
	menu_name ( "EditMenu",		"NewTransaction",	NULL ),
	menu_name ( "EditMenu",		"RemoveTransaction",	NULL ),
	menu_name ( "EditMenu",		"CloneTransaction",	NULL ),
	menu_name ( "EditMenu",		"EditTransaction",	NULL ),
	menu_name ( "EditMenu",		"ConvertToScheduled",	NULL ),
	menu_name ( "EditMenu",		"MoveToAnotherAccount",	NULL ),
	menu_name ( "EditMenu",		"Preferences",		NULL ),
	menu_name ( "EditMenu",		"RemoveAccount",	NULL ),
	menu_name ( "EditMenu",		"NewAccount",		NULL ),
	menu_name ( "ViewMenu",		"ShowTransactionForm",	NULL ),
	menu_name ( "ViewMenu",		"ShowGrid",		NULL ),
	menu_name ( "ViewMenu",		"ShowReconciled",	NULL ),
	menu_name ( "ViewMenu",		"ShowClosed",		NULL ),
	menu_name ( "ViewMenu",		"ShowOneLine",		NULL ),
	menu_name ( "ViewMenu",		"ShowTwoLines",		NULL ),
	menu_name ( "ViewMenu",		"ShowThreeLines",	NULL ),
	menu_name ( "ViewMenu",		"ShowFourLines",	NULL ),
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
	{0, 0, ELEMENT_CATEGORY, 0, ELEMENT_TYPE, ELEMENT_AMOUNT, 0 },
	{0, 0, ELEMENT_NOTES, 0, 0, 0, 0 },
	{0, 0, 0, 0, 0, 0, 0 }
    };
    gint i, j;

    devel_debug (NULL);

    for ( i = 0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	for ( j = 0 ; j<CUSTOM_MODEL_VISIBLE_COLUMNS ; j++ )
	    tab_affichage_ope[i][j] = tab[i][j];

    ligne_affichage_une_ligne = 0;

    if ( lignes_affichage_deux_lignes )
	g_slist_free ( lignes_affichage_deux_lignes );
    if ( lignes_affichage_trois_lignes )
	g_slist_free ( lignes_affichage_trois_lignes );

    lignes_affichage_deux_lignes = NULL;
    lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
						    NULL );
    lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
						    GINT_TO_POINTER (1));

    lignes_affichage_trois_lignes = NULL;
    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
						     NULL );
    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
						     GINT_TO_POINTER (1));
    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
						     GINT_TO_POINTER (2));
}
/*****************************************************************************************************/


  
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
