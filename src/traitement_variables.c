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
#include "echeancier_liste_constants.h"
#include "operations_formulaire_constants.h"
#include "traitement_variables_constants.h"



/*START_INCLUDE*/
#include "traitement_variables.h"
#include "gsb_account.h"
#include "gsb_transaction_data.h"
#include "menu.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void initialise_tab_affichage_ope ( void );
/*END_STATIC*/


/* les styles de couleur */

GtkStyle *style_couleur [2];
GtkStyle *style_rouge_couleur [2];
GtkStyle *style_gris;

GdkColor couleur_fond[2];
GdkColor couleur_grise;
GdkColor couleur_selection;
GSList *liste_labels_titres_colonnes_liste_ope = NULL;



/*START_EXTERN*/
extern gchar *adresse_commune;
extern gchar *adresse_secondaire;
extern gint affichage_echeances;
extern gint affichage_echeances_perso_j_m_a;
extern gint affichage_echeances_perso_nb_libre;
extern gchar *chemin_logo;
extern struct struct_devise *devise_compte;
extern struct struct_devise *devise_nulle;
extern struct struct_devise *devise_operation;
extern GSList *echeances_saisies;
extern struct struct_etat *etat_courant;
extern GtkItemFactory *item_factory_menu_general;
extern gint ligne_affichage_une_ligne;
extern GSList *lignes_affichage_deux_lignes;
extern GSList *lignes_affichage_trois_lignes;
extern GtkWidget *liste_categ_etat;
extern GSList *liste_struct_banques;
extern GSList *liste_struct_categories;
extern GSList *liste_struct_devises;
extern GSList *liste_struct_echeances;
extern GSList *liste_struct_etats;
extern GSList *liste_struct_exercices;
extern GSList *liste_struct_imputation;
extern GSList *liste_struct_rapprochements;
extern GSList *liste_struct_tiers;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern gint nb_banques;
extern gint nb_devises;
extern gint nb_echeances;
extern gint nb_enregistrements_categories, no_derniere_categorie;
extern gint nb_enregistrements_imputations;
extern gint nb_enregistrements_tiers;
extern gint no_dernier_etat;
extern gint no_dernier_tiers;
extern gint no_derniere_banque;
extern gint nb_enregistrements_categories, no_derniere_categorie;
extern gint no_derniere_devise;
extern gint no_derniere_echeance;
extern gint no_derniere_imputation;
extern gint no_derniere_operation;
extern int no_devise_totaux_categ;
extern gint no_devise_totaux_ib;
extern gint no_devise_totaux_tiers;
extern gchar *nom_fichier_backup;
extern gchar *nom_fichier_comptes;
extern gint scheduler_col_width[NB_COLS_SCHEDULER] ;
extern GtkWidget *solde_label;
extern GtkWidget *solde_label_pointe;
extern gint tab_affichage_ope[TRANSACTION_LIST_ROWS_NB][TRANSACTION_LIST_COL_NB];
extern gchar *titre_fichier;
extern GtkTooltips *tooltips_general_grisbi;
extern gint valeur_echelle_recherche_date_import;
extern GtkWidget *window;
/*END_EXTERN*/



/*****************************************************************************************************/
/* fonction appelée lors de modification ou non de fichier */
/* TRUE indique que le fichier a été modifié */
/* FALSE non */
/* ajuste la sensitive des menus en fonction */
/*****************************************************************************************************/

void modification_fichier ( gboolean modif )
{

    if ( modif )
    {
	if ( DEBUG )
	    printf ( "fichier modifie\n" );
	etat.modification_fichier = 1;
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Save"), NULL)),
				   TRUE );
    }
    else
    {
	if ( DEBUG )
	    printf ( "fichier non modifie\n" );
	etat.modification_fichier = 0;
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Save"), NULL)),
				   FALSE );
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

    if ( DEBUG )
	printf ( "init_variables\n" );

    gsb_account_init_variables ();
    gsb_transaction_data_init_variables ();

    mise_a_jour_liste_comptes_accueil = 0;
    mise_a_jour_liste_echeances_manuelles_accueil = 0;
    mise_a_jour_liste_echeances_auto_accueil = 0;
    mise_a_jour_soldes_minimaux = 0;
    mise_a_jour_fin_comptes_passifs = 0;

    etat.modification_fichier = 0;
    etat.utilise_logo = 1;

    nom_fichier_comptes = NULL;

    no_derniere_operation = 0;
    solde_label = NULL;
    solde_label_pointe = NULL;

    nom_fichier_backup = NULL;
    chemin_logo = NULL;

    liste_struct_echeances = NULL;
    nb_echeances = 0;
    no_derniere_echeance = 0;
    affichage_echeances = 3;
    affichage_echeances_perso_nb_libre = 0;
    affichage_echeances_perso_j_m_a = 0;
    echeances_saisies = NULL;

    liste_struct_tiers = NULL;
    nb_enregistrements_tiers = 0;
    no_dernier_tiers = 0;

    liste_struct_categories = NULL;
    nb_enregistrements_categories = 0;
    no_derniere_categorie = 0;

    liste_struct_imputation = NULL;
    nb_enregistrements_imputations = 0;
    no_derniere_imputation = 0;

    liste_struct_devises = NULL;
    nb_devises = 0;
    no_derniere_devise = 0;
    devise_nulle = calloc ( 1,
			    sizeof ( struct struct_devise ));
    no_devise_totaux_tiers = 1;
    no_devise_totaux_categ = 1;
    no_devise_totaux_ib = 1;
    devise_compte = NULL;
    devise_operation = NULL;

    liste_struct_banques = NULL;
    nb_banques = 0;
    no_derniere_banque = 0;


    liste_struct_rapprochements = NULL;

    titre_fichier = NULL;
    adresse_commune = NULL;
    adresse_secondaire = NULL;

    if ( liste_struct_exercices )
    {
	g_slist_free ( liste_struct_exercices );
	liste_struct_exercices = NULL;
    }

    liste_struct_etats = NULL;
    no_dernier_etat = 0;
    etat_courant = NULL;
    liste_categ_etat = NULL;

    initialise_tab_affichage_ope();

    etat.fichier_deja_ouvert = 0;
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

    /*     couleur grise de l'échéancier */

    couleur_grise.red = COULEUR_GRISE_RED;
    couleur_grise.green = COULEUR_GRISE_GREEN;
    couleur_grise.blue = COULEUR_GRISE_BLUE;

    /* initialise la couleur de la sélection */

    couleur_selection.red= COULEUR_SELECTION_RED;
    couleur_selection.green= COULEUR_SELECTION_GREEN ;
    couleur_selection.blue= COULEUR_SELECTION_BLUE;

    /* FIXME : plus besoin des styles... */
    /* initialise les variables style_couleur_1 et style_couleur_2 qui serviront */
    /* à mettre la couleur de fond */

    style_couleur [0] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
    style_couleur [0]->fg[GTK_STATE_NORMAL] = style_couleur [0]->black;
    style_couleur [0]->base[GTK_STATE_NORMAL] = couleur_fond[2];
    style_couleur [0]->fg[GTK_STATE_SELECTED] = style_couleur [0]->black;
    style_couleur [0]->bg[GTK_STATE_SELECTED] = couleur_selection;

    style_couleur [1] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
    style_couleur [1]->fg[GTK_STATE_NORMAL] = style_couleur [1]->black;
    style_couleur [1]->base[GTK_STATE_NORMAL] = couleur_fond[1];
    style_couleur [1]->fg[GTK_STATE_SELECTED] = style_couleur [1]->black;
    style_couleur [1]->bg[GTK_STATE_SELECTED] = couleur_selection;


    /* initialise les variables style_rouge_couleur [1] et style_rouge_couleur [2] qui serviront */
    /* à mettre en rouge le solde quand il est négatif */

    style_rouge_couleur [0] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
    style_rouge_couleur [0] ->fg[GTK_STATE_NORMAL] = couleur_rouge;
    style_rouge_couleur [0] ->base[GTK_STATE_NORMAL] = couleur_fond[2];
    style_rouge_couleur [0] ->fg[GTK_STATE_SELECTED] = couleur_rouge;
    style_rouge_couleur [0] ->bg[GTK_STATE_SELECTED] = couleur_selection;

    style_rouge_couleur [1] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
    style_rouge_couleur [1] ->fg[GTK_STATE_NORMAL] = couleur_rouge;
    style_rouge_couleur [1] ->base[GTK_STATE_NORMAL] = couleur_fond[1];
    style_rouge_couleur [1] ->fg[GTK_STATE_SELECTED] = couleur_rouge;
    style_rouge_couleur [1] ->bg[GTK_STATE_SELECTED] = couleur_selection;

    style_gris = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
    style_gris ->base[GTK_STATE_NORMAL] = couleur_grise;
    style_gris ->fg[GTK_STATE_NORMAL] = style_gris->black;

}
/*****************************************************************************************************/

/*****************************************************************************************************/
/* si grise = 1 ; grise tous les menus qui doivent l'être quand aucun fichier n'est en mémoire */
/* si grise = 0 ; dégrise les même menus */
/*****************************************************************************************************/

void menus_sensitifs ( gboolean sensitif )
{
    gchar * items[] = {
      menu_name(_("File"), _("Save"), NULL),
      menu_name(_("File"), _("Save as"), NULL),
      menu_name(_("File"), _("Export QIF file ..."), NULL),
      menu_name(_("File"), _("Close"), NULL),
      menu_name(_("Accounts"), _("Remove an account"), NULL),
      menu_name(_("Accounts"), _("Closed accounts"), NULL),
      menu_name(_("Edit"), _("New transaction"), NULL),
      menu_name(_("Edit"), _("Remove transaction"), NULL),
      menu_name(_("Edit"), _("Clone transaction"), NULL),
      menu_name(_("Edit"), _("Edit transaction"), NULL),
      menu_name(_("Edit"), _("Convert transaction to scheduled transaction"), NULL),
      menu_name(_("Edit"), _("Move transaction to another account"), NULL),
      menu_name(_("View"), _("Show transaction form"), NULL),
      menu_name(_("View"), _("Show grid"), NULL),
      menu_name(_("View"), _("Show reconciled transactions"), NULL),
      menu_name(_("View"), _("Show one line per transaction"), NULL),
      menu_name(_("View"), _("Show two lines per transaction"), NULL),
      menu_name(_("View"), _("Show three lines per transaction"), NULL),
      menu_name(_("View"), _("Show four lines per transaction"), NULL),
      menu_name(_("Reports"), _("New report"), NULL),
      menu_name(_("Reports"), _("Clone report"), NULL),
      menu_name(_("Reports"), _("Print report"), NULL),
      menu_name(_("Reports"), _("Import report"), NULL),
      menu_name(_("Reports"), _("Export report"), NULL),
      menu_name(_("Reports"), _("Remove report"), NULL),
      menu_name(_("Reports"), _("Edit report"), NULL),
      NULL,
    };
    gchar ** tmp = items;

    if ( DEBUG )
	printf ( "menus_sensitifs : %d\n", sensitif );

    while ( *tmp )
      {
	  if ( gtk_item_factory_get_item ( item_factory_menu_general,
					   *tmp ))
	      gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
								     *tmp ),
					 sensitif );
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
	    if ( !gsb_account_get_column_sort ( no_account,
						j )
		 &&
		 tab_affichage_ope[i][j]
		 &&
		 tab_affichage_ope[i][j] != TRANSACTION_LIST_BALANCE )
		gsb_account_set_column_sort ( no_account,
					      j,
					      tab_affichage_ope[i][j] );
	}

    /* the default sort is by date and ascending */

    gsb_account_set_sort_type ( no_account,
				GTK_SORT_ASCENDING );
    gsb_account_set_sort_column ( no_account,
				  TRANSACTION_COL_NB_DATE );
}


  
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
