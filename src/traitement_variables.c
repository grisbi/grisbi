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


extern gint valeur_echelle_recherche_date_import;
extern GtkItemFactory *item_factory_menu_general;
extern GSList *gsliste_echeances; 
extern gint nb_echeances;
extern gint no_derniere_echeance;
extern gint affichage_echeances; 
extern gint affichage_echeances_perso_nb_libre; 
extern gint affichage_echeances_perso_j_m_a; 
extern GSList *echeances_saisies;


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
	etat.modification_fichier = 1;
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Save"), NULL)),
				   TRUE );
    }
    else
    {
	etat.modification_fichier = 0;
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Save"), NULL)),
				   FALSE );
    }

}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* fonction appelée à chaque ouverture de fichier et qui initialise les variables globales */
/* si ouverture = TRUE, la fonction considère qu'on est en train d'ouvrir un fichier et ajuste les menus */
/* en conséquence */
/* sinon, suppose que fermeture de fichier */
/*****************************************************************************************************/

void init_variables ( gboolean ouverture )
{

    if ( ouverture )
    {
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Save as"), NULL)),
				   TRUE );
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Export"), NULL)),
				   TRUE );
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Close"), NULL)),
				   TRUE );
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("Accounts"), _("Remove an account"), NULL)),
				   TRUE );

	/* remplit les tabeaux tips_col_liste_operations et titres_colonnes_liste_operations */

	recuperation_noms_colonnes_et_tips();
    }
    else
    {
	etat.modification_fichier = 0;
	etat.utilise_logo = 1;

	nom_fichier_comptes = NULL;

	nb_comptes = 0;
	no_derniere_operation = 0;
	p_tab_nom_de_compte = NULL;
	ordre_comptes = NULL;
	compte_courant = 0;
	solde_label = NULL;
	etat.ancienne_date = 0;
	p_tab_nom_de_compte = NULL;

	nom_fichier_backup = NULL;
	chemin_logo = NULL;

	gsliste_echeances = NULL;
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

	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Save"), NULL)),
				   FALSE );
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Save as"), NULL)),
				   FALSE );
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Export"), NULL)),
				   FALSE );
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("File"), _("Close"), NULL)),
				   FALSE );
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("Accounts"), _("Remove an account"), NULL)),
				   FALSE );
	gtk_widget_set_sensitive ( gtk_item_factory_get_item ( item_factory_menu_general,
							       menu_name(_("Accounts"), _("Closed accounts"), NULL)),
				   FALSE );


	liste_no_rapprochements = NULL;

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

    }
}
/*****************************************************************************************************/


/*****************************************************************************************************/
void initialise_tab_affichage_ope ( void )
{
    gint tab[4][7] = { { 18, 1, 3, 13, 5, 6, 7 },
	{0, 0, 12, 0, 9, 8, 0 },
	{0, 11, 15, 0, 0, 0, 0 },
	{0, 0, 0, 0, 0, 0, 0 }};
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
void initialisation_couleurs_listes ( void )
{
    GdkColor couleur1;
    GdkColor couleur2;
    GdkColor couleur_rouge;
    GdkColor couleur_selection;
    GdkColor couleur_grise;

    /* Initialisation des couleurs de la clist */

    couleur1.red = COULEUR1_RED ;
    couleur1.green = COULEUR1_GREEN;
    couleur1.blue = COULEUR1_BLUE;

    couleur2.red = COULEUR2_RED;
    couleur2.green = COULEUR2_GREEN;
    couleur2.blue = COULEUR2_BLUE;

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


    /* initialise les variables style_couleur_1 et style_couleur_2 qui serviront */
    /* à mettre la couleur de fond */

    style_couleur [0] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
    style_couleur [0]->fg[GTK_STATE_NORMAL] = style_couleur [0]->black;
    style_couleur [0]->base[GTK_STATE_NORMAL] = couleur2;
    style_couleur [0]->fg[GTK_STATE_SELECTED] = style_couleur [0]->black;
    style_couleur [0]->bg[GTK_STATE_SELECTED] = couleur_selection;

    style_couleur [1] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
    style_couleur [1]->fg[GTK_STATE_NORMAL] = style_couleur [1]->black;
    style_couleur [1]->base[GTK_STATE_NORMAL] = couleur1;
    style_couleur [1]->fg[GTK_STATE_SELECTED] = style_couleur [1]->black;
    style_couleur [1]->bg[GTK_STATE_SELECTED] = couleur_selection;


    /* initialise les variables style_rouge_couleur [1] et style_rouge_couleur [2] qui serviront */
    /* à mettre en rouge le solde quand il est négatif */

    style_rouge_couleur [0] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
    style_rouge_couleur [0] ->fg[GTK_STATE_NORMAL] = couleur_rouge;
    style_rouge_couleur [0] ->base[GTK_STATE_NORMAL] = couleur2;
    style_rouge_couleur [0] ->fg[GTK_STATE_SELECTED] = couleur_rouge;
    style_rouge_couleur [0] ->bg[GTK_STATE_SELECTED] = couleur_selection;

    style_rouge_couleur [1] = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
    style_rouge_couleur [1] ->fg[GTK_STATE_NORMAL] = couleur_rouge;
    style_rouge_couleur [1] ->base[GTK_STATE_NORMAL] = couleur1;
    style_rouge_couleur [1] ->fg[GTK_STATE_SELECTED] = couleur_rouge;
    style_rouge_couleur [1] ->bg[GTK_STATE_SELECTED] = couleur_selection;

    style_gris = gtk_style_copy ( gtk_widget_get_style (GTK_WIDGET (window)) );
    style_gris ->base[GTK_STATE_NORMAL] = couleur_grise;
    style_gris ->fg[GTK_STATE_NORMAL] = style_gris->black;

}
/*****************************************************************************************************/

