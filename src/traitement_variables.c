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
#include "structures.h"
#include "variables-extern.c"

#include "affichage_liste.h"
#include "gtkcombofix.h"
#include "menu.h"
#include "traitement_variables.h"
#include "utils.h"
#include "affichage.h"
#include "affichage_formulaire.h"

static void initialise_tab_affichage_ope ( void );



GdkColor couleur_fond[2];
GdkColor couleur_selection;
GSList *liste_labels_titres_colonnes_liste_ope = NULL;

gchar *labels_titres_colonnes_liste_ope[] = {
    N_("Date"),
    N_("Value date"),
    N_("Third party"),
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
    N_("Cheque/Transfer number"),
    NULL };


extern gint valeur_echelle_recherche_date_import;
extern GtkItemFactory *item_factory_menu_general;
extern GSList *liste_struct_echeances; 
extern gint nb_echeances;
extern gint no_derniere_echeance;
extern gint affichage_echeances; 
extern gint affichage_echeances_perso_nb_libre; 
extern gint affichage_echeances_perso_j_m_a; 
extern GSList *echeances_saisies;

extern GtkTooltips *tooltips_general_grisbi;
extern GtkTreeViewColumn *colonnes_liste_ventils[3];
extern GSList *liste_struct_banques;
extern gint nb_banques;
extern gint no_derniere_banque;
extern GSList *liste_struct_rapprochements;
extern GSList *liste_struct_imputation;
extern gint nb_enregistrements_imputations;
extern gint no_derniere_imputation;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_liste_echeances_manuelles_accueil;
extern gint mise_a_jour_liste_echeances_auto_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint mise_a_jour_combofix_tiers_necessaire;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint mise_a_jour_combofix_imputation_necessaire;
extern struct organisation_formulaire *organisation_generale_formulaire;




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

    if ( DEBUG )
	printf ( "init_variables\n" );

    mise_a_jour_liste_comptes_accueil = 0;
    mise_a_jour_liste_echeances_manuelles_accueil = 0;
    mise_a_jour_liste_echeances_auto_accueil = 0;
    mise_a_jour_soldes_minimaux = 0;
    mise_a_jour_fin_comptes_passifs = 0;

    etat.modification_fichier = 0;
    etat.utilise_logo = 1;

    nom_fichier_comptes = NULL;

    nb_comptes = 0;
    no_derniere_operation = 0;
    p_tab_nom_de_compte = NULL;
    ordre_comptes = NULL;
    compte_courant = 0;
    solde_label = NULL;
    p_tab_nom_de_compte = NULL;

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
    devise_compte = NULL;
    devise_operation = NULL;

    liste_struct_banques = NULL;
    nb_banques = 0;
    no_derniere_banque = 0;


    liste_struct_rapprochements = NULL;

    titre_fichier = NULL;
    adresse_commune = NULL;

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

    for ( i=0 ; i<3 ; i++ )
	colonnes_liste_ventils[i] = NULL;

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

}
/*****************************************************************************************************/





/*****************************************************************************************************/
void initialisation_couleurs_listes ( void )
{
    GdkColor couleur_rouge;
    GdkColor couleur_grise;

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

    if ( DEBUG )
	printf ( "menus_sensitifs : %d\n", sensitif );

    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("File"), _("Save"), NULL)),
			       sensitif );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("File"), _("Save as"), NULL)),
			       sensitif );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("File"), _("Export"), NULL)),
			       sensitif );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("File"), _("Close"), NULL)),
			       sensitif );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Accounts"), _("Remove an account"), NULL)),
			       sensitif );
    gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							   menu_name(_("Accounts"), _("Closed accounts"), NULL)),
			       sensitif );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
void initialise_tab_affichage_ope ( void )
{
    gint tab[4][7] = {
	{ TRANSACTION_LIST_CHQ, TRANSACTION_LIST_DATE, TRANSACTION_LIST_PARTY, TRANSACTION_LIST_MARK, TRANSACTION_LIST_CREDIT, TRANSACTION_LIST_DEBIT, TRANSACTION_LIST_BALANCE },
	{0, 0, TRANSACTION_LIST_CATEGORY, 0, TRANSACTION_LIST_TYPE, TRANSACTION_LIST_AMOUNT, 0 },
	{0, 0, TRANSACTION_LIST_NOTES, 0, 0, 0, 0 },
	{0, 0, 0, 0, 0, 0, 0 }
    };
    gint i, j;

    for ( i = 0 ; i<4 ; i++ )
	for ( j = 0 ; j<7 ; j++ )
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


/*****************************************************************************************************/
/* cette fonction, appelée par l'initialisation des variables globales remet l'organisation de */
/* formulaire à zéro : */
/* date		tiers	débit		crédit 	*/
/* 		catég	mode paiement	chq 	*/
/* ventil	notes	mode contre op		*/
/*****************************************************************************************************/
struct organisation_formulaire *mise_a_zero_organisation_formulaire ( void )
{
    struct organisation_formulaire *struct_formulaire;
    gint tab[4][6] = { 
	{ TRANSACTION_FORM_DATE, TRANSACTION_FORM_PARTY, TRANSACTION_FORM_DEBIT, TRANSACTION_FORM_CREDIT, 0, 0 },
	{ 0, TRANSACTION_FORM_CATEGORY, TRANSACTION_FORM_TYPE, TRANSACTION_FORM_CHEQUE, 0, 0 },
	{ TRANSACTION_FORM_BREAKDOWN, TRANSACTION_FORM_NOTES, TRANSACTION_FORM_CONTRA, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0 }
    };
    gint i, j;
    gint taille[6] = { 15, 50, 15, 15, 0, 0 };

    struct_formulaire = malloc ( sizeof ( struct organisation_formulaire ));
    
    struct_formulaire -> nb_colonnes = 4;
    struct_formulaire -> nb_lignes = 3;

    for ( i = 0 ; i<4 ; i++ )
	for ( j = 0 ; j<6 ; j++ )
	    struct_formulaire -> tab_remplissage_formulaire[i][j] = tab[i][j];

    for ( i = 0 ; i<6 ; i++ )
	struct_formulaire -> taille_colonne_pourcent[i] = taille[i];

    return struct_formulaire;
}
/*****************************************************************************************************/

