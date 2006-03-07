/* Fichier traitement_variables.c */
/* Contient toutes les procédures relatives au traitement des variables */

/*     Copyright (C) 2000-2003  Cédric Auger */
/* 			cedric@grisbi.org */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


#include "include.h"



/*START_INCLUDE*/
#include "traitement_variables.h"
#include "erreur.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_budget.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_currency_link.h"
#include "gsb_data_fyear.h"
#include "gsb_data_payee.h"
#include "gsb_data_report_amout_comparison.h"
#include "gsb_data_report.h"
#include "gsb_data_report_text_comparison.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_fyear.h"
#include "menu.h"
#include "structures.h"
#include "traitement_variables.h"
#include "gsb_scheduler_list.h"
#include "include.h"
#include "echeancier_infos.h"
#include "gsb_transactions_list.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void initialise_tab_affichage_ope ( void );
/*END_STATIC*/

gchar *labels_titres_colonnes_liste_ope[] = {
    N_("Date"),
    N_("Value date"),
    N_("Payee"),
    N_("Budgetary lines"),
    N_("Credit"),
    N_("Debit"),
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
GdkColor couleur_selection;
GdkColor breakdown_background;


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

GSList *liste_labels_titres_colonnes_liste_ope = NULL;

gchar *nom_fichier_comptes;

gchar *titre_fichier;
gchar *adresse_commune;
gchar *adresse_secondaire;



/*START_EXTERN*/
extern gint affichage_echeances;
extern gint affichage_echeances_perso_nb_libre;
extern gchar *chemin_logo;
extern gchar *crypt_key;
extern gint ligne_affichage_une_ligne;
extern GSList *lignes_affichage_deux_lignes;
extern GSList *lignes_affichage_trois_lignes;
extern GtkWidget *liste_categ_etat;
extern GSList *liste_struct_banques;
extern GSList *liste_struct_rapprochements;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern gint nb_banques;
extern gint no_derniere_banque;
extern int no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern gchar *nom_fichier_backup;
extern GSList *scheduled_transactions_taken;
extern gint scheduler_col_width[NB_COLS_SCHEDULER];
extern GtkWidget *solde_label ;
extern GtkWidget *solde_label_pointe ;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
extern GtkTooltips *tooltips_general_grisbi;
extern gint valeur_echelle_recherche_date_import;
/*END_EXTERN*/



/*****************************************************************************************************/
/* fonction appelée lors de modification ou non de fichier */
/* TRUE indique que le fichier a été modifié */
/* FALSE non */
/* ajuste la sensitive des menus en fonction */
/*****************************************************************************************************/

void modification_fichier ( gboolean modif )
{
    devel_debug ( g_strdup_printf ("modification_fichier : %d", modif ));

    /* If no file is loaded, do not change menu items. */
    if ( ! gsb_data_account_get_accounts_amount () )
    {
	return;
    }

    if ( modif )
    {
	etat.modification_fichier = 1;
	gsb_gui_sensitive_menu_item ( "FileMenu", "Save", NULL, TRUE );
    }
    else
    {
	etat.modification_fichier = 0;
	gsb_gui_sensitive_menu_item ( "FileMenu", "Save", NULL, FALSE );
    }
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* fonction  qui initialise les variables globales */
/*****************************************************************************************************/

void init_variables ( void )
{
    gint i;
    gint scheduler_col_width_init[NB_COLS_SCHEDULER] = { 10, 26, 20, 14, 14, 28, 8};

    devel_debug ( "init_variables" );

    gsb_data_account_init_variables ();
    gsb_data_transaction_init_variables ();
    gsb_data_payee_init_variables ();
    gsb_data_category_init_variables ();
    gsb_data_budget_init_variables ();
    gsb_data_report_init_variables ();
    gsb_data_report_amount_comparison_init_variables ();
    gsb_data_report_text_comparison_init_variables ();
    gsb_data_scheduled_init_variables ();
    gsb_data_currency_init_variables ();
    gsb_data_currency_link_init_variables ();
    gsb_data_fyear_init_variables ();

    gsb_currency_init_variables ();
    gsb_fyear_init_variables ();
    crypt_key = NULL;

    mise_a_jour_liste_comptes_accueil = 0;
    mise_a_jour_liste_echeances_manuelles_accueil = 0;
    mise_a_jour_liste_echeances_auto_accueil = 0;
    mise_a_jour_soldes_minimaux = 0;
    mise_a_jour_fin_comptes_passifs = 0;

    nom_fichier_comptes = NULL;

    solde_label = NULL;
    solde_label_pointe = NULL;

    nom_fichier_backup = NULL;
    chemin_logo = NULL;

    affichage_echeances = SCHEDULER_PERIODICITY_ONCE_VIEW;
    affichage_echeances_perso_nb_libre = 0;
    affichage_echeances_perso_j_m_a = PERIODICITY_DAYS;
    scheduled_transactions_taken = NULL;

    no_devise_totaux_tiers = 1;
    no_devise_totaux_categ = 1;
    no_devise_totaux_ib = 1;

    liste_struct_banques = NULL;
    nb_banques = 0;
    no_derniere_banque = 0;


    liste_struct_rapprochements = NULL;

    titre_fichier = _("My accounting");
    adresse_commune = NULL;
    adresse_secondaire = NULL;

    liste_categ_etat = NULL;

    initialise_tab_affichage_ope();

    valeur_echelle_recherche_date_import = 2;

    tooltips_general_grisbi = NULL;

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
    mise_a_jour_combofix_tiers_necessaire = 0;
    mise_a_jour_combofix_categ_necessaire = 0;
    mise_a_jour_combofix_imputation_necessaire = 0;

    /*     on met à jour les valeurs par défaut des largeurs de colonnes de la liste d'échéances */

    for ( i = 0 ; i < NB_COLS_SCHEDULER ; i++ )
	scheduler_col_width[i] = scheduler_col_width_init[i];
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

    /* color for breakdown children */

    breakdown_background.red = BREAKDOWN_BACKGROUND_COLOR_RED;
    breakdown_background.green = BREAKDOWN_BACKGROUND_COLOR_GREEN;
    breakdown_background.blue = BREAKDOWN_BACKGROUND_COLOR_BLUE;
    breakdown_background.pixel = 0;

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
	menu_name ( "FileMenu",		"ExportFile",		NULL ),
	menu_name ( "FileMenu",		"Close",		NULL ),
	menu_name ( "EditMenu",		"NewTransaction",	NULL ),
	menu_name ( "EditMenu",		"RemoveTransaction",	NULL ),
	menu_name ( "EditMenu",		"CloneTransaction",	NULL ),
	menu_name ( "EditMenu",		"EditTransaction",	NULL ),
	menu_name ( "EditMenu",		"ConvertToScheduled",	NULL ),
	menu_name ( "EditMenu",		"MoveToAnotherAccount",	NULL ),
	menu_name ( "EditMenu",		"Preferences",		NULL ),
	menu_name ( "EditMenu",		"RemoveAccount",	NULL ),
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
    
    devel_debug ( g_strdup_printf ("menus_sensitifs : %d", sensitif ));

    while ( *tmp )
    {
	gsb_gui_sensitive_menu_item_from_string ( *tmp, sensitif );
	tmp++;
    }
}
/*****************************************************************************************************/




/*****************************************************************************************************/
void initialise_tab_affichage_ope ( void )
{
    gint tab[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB] = {
	{ TRANSACTION_LIST_CHQ, TRANSACTION_LIST_DATE, TRANSACTION_LIST_PARTY, TRANSACTION_LIST_MARK, TRANSACTION_LIST_DEBIT, TRANSACTION_LIST_CREDIT, TRANSACTION_LIST_BALANCE },
	{0, 0, TRANSACTION_LIST_CATEGORY, 0, TRANSACTION_LIST_TYPE, TRANSACTION_LIST_AMOUNT, 0 },
	{0, 0, TRANSACTION_LIST_NOTES, 0, 0, 0, 0 },
	{0, 0, 0, 0, 0, 0, 0 }
    };
    gint i, j;

    devel_debug ("initialise_tab_affichage_ope");

    for ( i = 0 ; i<TRANSACTION_LIST_ROWS_NB ; i++ )
	for ( j = 0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
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

/** set the default column_sort for each account when loading a file before 0.6.0
 * or make a new file
 * \param no_account
 * \return
 * */
void init_default_sort_column ( gint no_account )
{
    gint i, j;

    for ( i = 0 ; i<4 ; i++ )
	for ( j = 0 ; j<TRANSACTION_LIST_COL_NB ; j++ )
	{
	    if ( !gsb_data_account_get_column_sort ( no_account,
						j )
		 &&
		 tab_affichage_ope[i][j]
		 &&
		 tab_affichage_ope[i][j] != TRANSACTION_LIST_BALANCE )
		gsb_data_account_set_column_sort ( no_account,
					      j,
					      tab_affichage_ope[i][j] );
	}

    /* the default sort is by date and ascending */

    gsb_data_account_set_sort_type ( no_account,
				GTK_SORT_ASCENDING );
    gsb_data_account_set_sort_column ( no_account,
				  TRANSACTION_COL_NB_DATE );
}


  
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
