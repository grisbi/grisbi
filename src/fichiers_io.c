/* Fichier fichiers_io.c */
/* Contient toutes les procédures relatives à l'accès au disque */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org) */
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

/* structure utilisée pour récupérer les nos de versions */

struct recuperation_version
{
    gchar *version_fichier;
    gchar *version_grisbi;
};



/*START_INCLUDE*/
#include "fichiers_io.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "utils_devises.h"
#include "dialog.h"
#include "utils_str.h"
#include "main.h"
#include "traitement_variables.h"
#include "utils_buttons.h"
#include "fichiers_gestion.h"
#include "search_glist.h"
#include "operations_liste.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean charge_categ_version_0_4_0 ( xmlDocPtr doc );
static gboolean charge_fichier_xml_grisbi ( xmlDocPtr doc );
static gboolean charge_ib_version_0_4_0 ( xmlDocPtr doc );
static gboolean mise_a_jour_versions_anterieures ( gint no_version,
					    struct recuperation_version *version );
static void propose_changement_permissions ( void );
static gboolean recuperation_banques_xml ( xmlNodePtr node_banques );
static gboolean recuperation_categories_xml ( xmlNodePtr node_categories );
static gboolean recuperation_comptes_xml ( xmlNodePtr node_comptes );
static gboolean recuperation_devises_xml ( xmlNodePtr node_devises );
static gboolean recuperation_echeances_xml ( xmlNodePtr node_echeances );
static gboolean recuperation_etats_xml ( xmlNodePtr node_etats );
static gboolean recuperation_exercices_xml ( xmlNodePtr node_exercices );
static gboolean recuperation_generalites_xml ( xmlNodePtr node_generalites );
static gboolean recuperation_imputations_xml ( xmlNodePtr node_imputations );
static gboolean recuperation_rapprochements_xml ( xmlNodePtr node_rapprochements );
static gboolean recuperation_tiers_xml ( xmlNodePtr node_tiers );
static struct recuperation_version *recupere_version_fichier ( xmlDocPtr doc );
static void switch_t_r ( void );
/*END_STATIC*/





gchar *nom_fichier_comptes;

/* pointe vers un tableau de pointeurs vers les comptes en mémoire*/

gpointer **p_tab_nom_de_compte;

/* idem, mais utilisé pour se déplacer */

gpointer **p_tab_nom_de_compte_variable;


gchar *titre_fichier;
gchar *adresse_commune;
gchar *adresse_secondaire;

/* contient le dernier numéro d'opération de tous les comptes réunis */

gint no_derniere_operation;
gint nb_comptes;



/*START_EXTERN*/
extern GtkWidget *adr_banque;
extern gint affichage_echeances;
extern gint affichage_echeances_perso_j_m_a;
extern gint affichage_echeances_perso_nb_libre;
extern gchar *chemin_logo;
extern GtkWidget *code_banque;
extern gint compte_courant;
extern GtkWidget *email_banque;
extern GtkWidget *email_correspondant;
extern GtkWidget *fax_correspondant;
extern GtkWidget *formulaire;
extern gint ligne_affichage_une_ligne;
extern GSList *lignes_affichage_deux_lignes;
extern GSList *lignes_affichage_trois_lignes;
extern GSList *liste_struct_banques;
extern GSList *liste_struct_categories;
extern GSList *liste_struct_devises;
extern GSList *liste_struct_echeances;
extern GSList *liste_struct_etats;
extern GSList *liste_struct_exercices;
extern GSList *liste_struct_imputation;
extern GSList *liste_struct_rapprochements;
extern GSList *liste_struct_tiers;
extern gint nb_banques;
extern gint nb_colonnes;
extern gint nb_devises;
extern gint nb_echeances;
extern gint nb_enregistrements_categories;
extern gint nb_enregistrements_imputations;
extern gint nb_enregistrements_tiers;
extern gint nb_exercices;
extern gint no_dernier_etat;
extern gint no_dernier_tiers;
extern gint no_derniere_banque;
extern gint no_derniere_categorie;
extern gint no_derniere_devise;
extern gint no_derniere_echeance;
extern gint no_derniere_exercice;
extern gint no_derniere_imputation;
extern gint no_devise_totaux_tiers;
extern GtkWidget *nom_banque;
extern GtkWidget *nom_correspondant;
extern GtkWidget *nom_exercice;
extern gchar *nom_fichier_backup;
extern GSList *ordre_comptes;
extern gint rapport_largeur_colonnes[7];
extern GtkWidget *remarque_banque;
extern gint scheduler_col_width[NB_COLS_SCHEDULER] ;
extern GtkTreeSelection * selection;
extern gint tab_affichage_ope[4][7];
extern GtkWidget *tel_banque;
extern GtkWidget *tel_correspondant;
extern gint valeur_echelle_recherche_date_import;
extern GtkWidget *web_banque;
extern GtkWidget *window;
/*END_EXTERN*/




/****************************************************************************/
/** Procédure qui charge les opérations en mémoire sous forme de structures**/
/****************************************************************************/

gboolean charge_operations ( gchar *nom_fichier )
{
    struct stat buffer_stat;
    xmlDocPtr doc;
    int result;
    struct recuperation_version *version;

    if ( DEBUG )
	printf ( "charge_operations\n" );

    /* vérification de la permission du fichier */

    result = stat ( nom_fichier, &buffer_stat);

    if ( result != -1 && 
	 buffer_stat.st_mode != 33152 && !etat.display_message_file_readable )
	propose_changement_permissions();


    /* on commence par ouvrir le fichier en xml */

    if ( result != -1 )
    {
	doc = xmlParseFile ( nom_fichier );

	if ( doc )
	{
	    gint no_version;

	    /* récupère la version de fichier */

	    version = recupere_version_fichier ( doc );

	    /* 	    si les versions ne sont pas récupérées, c'est que ce n'est pas un fichier grisbi */

	    if ( !version -> version_fichier
		 ||
		 !version -> version_grisbi )
	    {
		dialogue_error ( _("Invalid accounts file") );
		xmlFreeDoc ( doc );
		return ( FALSE );
	    }

	    /* 	    on transforme la version en integer */

	    no_version = my_atoi ( g_strjoinv ( "",
						g_strsplit ( version -> version_fichier,
							     ".",
							     0 )));


	    /* 	    on va commencer par charger le fichier */
	    /* 		théoriquement toute version peut être chargée, même une version */
	    /* 		du futur, la seule chose, c'est qu'on le remarquera ensuite et */
	    /* 		on ne le démarrera pas */

	    charge_fichier_xml_grisbi ( doc );

	    /* 	    on effectue les modif en fonction de la version du fichier */
	    /* 		et retourne TRUE si ça s'est bien passé */

	    return (mise_a_jour_versions_anterieures ( no_version,
						       version ));

	}

	dialogue_error (g_strdup_printf (_("Cannot open file '%s': %s"), nom_fichier,
					 latin2utf8 (strerror(errno))));
	remove_file_from_last_opened_files_list ( nom_fichier );
	return ( FALSE );
    }
    else
    {
	dialogue_error (g_strdup_printf (_("Cannot open file '%s': %s"), nom_fichier,
					 latin2utf8 (strerror(errno))));
	remove_file_from_last_opened_files_list ( nom_fichier );
	return FALSE;
    }
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* renvoie la version du fichier et de grisbi qui a fait le fichier */
/* les strings sont à null si non récupérées */
/***********************************************************************************************************/
struct recuperation_version *recupere_version_fichier ( xmlDocPtr doc )
{
    struct recuperation_version *version;
    xmlNodePtr node_1;
    xmlNodePtr root = xmlDocGetRootElement(doc);


    version = calloc ( 1,
		       sizeof ( struct recuperation_version ));

    node_1 = root -> children;

    while ( node_1 )
    {
	/* les versions de fichier et de grisbi font partie des généralités */

	if ( !strcmp ( node_1 -> name,
		       "Generalites" ))
	{
	     xmlNodePtr node_generalites;

	     node_generalites = node_1 -> children;

	     while ( node_generalites )
	     {
		 if ( !strcmp ( node_generalites -> name,
				"Version_fichier" ))
		     version -> version_fichier = xmlNodeGetContent ( node_generalites );
		 if ( !strcmp ( node_generalites -> name,
				"Version_grisbi" ))
		     version -> version_grisbi  = xmlNodeGetContent ( node_generalites );

		 node_generalites = node_generalites -> next;
	     }
	}
	node_1 = node_1 -> next;
    }
    return version;
}
/*****************************************************************************/




/*****************************************************************************/
/* cette fonction rajoute les modifications qui ont été faite en fonction des différentes */
/* versions */
/*****************************************************************************/
gboolean mise_a_jour_versions_anterieures ( gint no_version,
					    struct recuperation_version *version )
{
    gint i;
    struct struct_devise *devise;
    struct stat buffer_stat;

    /*     par défaut le fichier n'est pas modifié sauf si on charge une version précédente */

    modification_fichier ( FALSE );

    switch ( no_version )
    {
	/* ************************************* */
	/*     ouverture d'un fichier 0.4.0      */
	/* ************************************* */

	case 40:

	    /* il n'y a aucune différence de struct entre la 0.4.0 et la 0.4.1 */
	    /* sauf que la 0.4.0 n'attribuait pas le no de relevé aux opés filles */
	    /* d'une ventilation */

	    for ( i = 0 ; i < nb_comptes ; i++ )
	    {
		GSList *liste_tmp;

		p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

		liste_tmp = LISTE_OPERATIONS;

		while ( liste_tmp )
		{
		    struct structure_operation *operation;

		    operation = liste_tmp -> data;

		    /*  si l'opération est une ventil, on refait le tour de la liste pour trouver ses filles */

		    if ( operation -> operation_ventilee )
		    {
			GSList *liste_tmp_2;

			liste_tmp_2 = LISTE_OPERATIONS;

			while ( liste_tmp_2 )
			{
			    struct structure_operation *operation_2;

			    operation_2 = liste_tmp_2 -> data;

			    if ( operation_2 -> no_operation_ventilee_associee == operation -> no_operation )
				operation_2 -> no_rapprochement = operation -> no_rapprochement;

			    liste_tmp_2 = liste_tmp_2 -> next;
			}
		    }
		    liste_tmp = liste_tmp -> next;
		}
	    }


	    /* ************************************* */
	    /* 	    ouverture d'un fichier 0.4.1     */
	    /* ************************************* */

	case 41:

	    /*     ajout de la 0.5 -> valeur_echelle_recherche_date_import qu'on me à 2 */

	    valeur_echelle_recherche_date_import = 2;

	    /* 	    passage à l'utf8 : on fait le tour des devises pour retrouver l'euro */
	    /* Handle Euro nicely */

	    devise = devise_par_nom ( g_strdup ("Euro"));

	    if ( devise )
	    {
		devise -> code_devise = "€";
		devise -> code_iso4217_devise = g_strdup ("EUR");
	    }


	    /* ************************************* */
	    /* 	    ouverture d'un fichier 0.5.0     */
	    /* ************************************* */

	case 50:
	    /* pour l'instant le fichier 0.5.1 ne diffère pas de la version 0.5.0 */
	    /*     excepté un changement dans la notation du pointage */
	    /*     rien=0 ; P=1 ; T=2 ; R=3 */
	    /*     on fait donc le tour des opés pour inverser R et P */

	    switch_t_r ();

	    /* 	    on met l'organisation des formulaires de tous les comptes à 0 */

	    for ( i=0 ; i<nb_comptes ; i++ )
	    {
		p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		ORGANISATION_FORMULAIRE = mise_a_zero_organisation_formulaire(); 
	    }

	    /* 	    un bug dans la 0.5.0 permettait à des comptes d'avoir un affichage différent, */
	    /* 	    même si celui ci devait être identique pour tous, on vérifie ici */

	    if ( !etat.retient_affichage_par_compte )
	    {
		gint affichage_r;
		gint nb_lignes_ope;
		gint i;

		p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

		affichage_r = AFFICHAGE_R;
		nb_lignes_ope = NB_LIGNES_OPE;

		for ( i=0 ; i<nb_comptes ; i++ )
		{
		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

		    AFFICHAGE_R = affichage_r;
		    NB_LIGNES_OPE = nb_lignes_ope;
		}
	    } 

	    /* 	    on met le classement courant par date et ordre croissant */

	    for ( i=0 ; i<nb_comptes ; i++ )
	    {
		p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		NO_CLASSEMENT = TRANSACTION_LIST_DATE;
		CLASSEMENT_CROISSANT = GTK_SORT_DESCENDING;
		CLASSEMENT_COURANT = recupere_classement_par_no ( NO_CLASSEMENT );
	    }



	    /* ********************************************************* */
	    /* 	    à mettre à chaque fois juste avant la version stable */
	    /* ********************************************************* */

	    modification_fichier ( TRUE );
	    

	    /* ************************************* */
	    /* 	    ouverture d'un fichier 0.5.1     */
	    /* ************************************* */

	case 51:



	    break;

	    /* ************************************* */
	    /* 	    ouverture d'un fichier 0.6.0     */
	    /* ************************************* */

	case 60:



	    break;

	default :
	    /* 	à ce niveau, c'est que que la version n'est pas connue de grisbi, on donne alors */
	    /* la version nécessaire pour l'ouvrir */

	    dialogue_error ( g_strdup_printf ( _("Grisbi version %s is needed to open this file.\nYou are using version %s."),
					       version -> version_fichier,
					       version -> version_grisbi ));

	    return ( FALSE );
    }

    /*     on met maintenant les généralités pour toutes les versions */

    /* 	s'il y avait un ancien logo mais qu'il n'existe plus, on met le logo par défaut */

    if ( !chemin_logo
	 ||
	 !strlen ( chemin_logo )
	 ||
	 ( chemin_logo
	   &&
	   strlen ( chemin_logo )
	   &&
	   stat ( chemin_logo, &buffer_stat) == -1 ))
	chemin_logo = g_strdup ( LOGO_PATH );

    /* on marque le fichier comme ouvert */

    modification_etat_ouverture_fichier ( TRUE );


    return TRUE;
}
/*****************************************************************************/





/*****************************************************************************/
/* charge la structure générale du fichier de grisbi, quelle que soit la version */
/*****************************************************************************/
gboolean charge_fichier_xml_grisbi ( xmlDocPtr doc )
{
    xmlNodePtr node_1;
    xmlNodePtr root = xmlDocGetRootElement(doc);

    etat.en_train_de_charger = 1;

    /* on place node_1 sur les généralités */

    node_1 = root -> children;

    /*   on met en place la boucle de node_1, qui va successivement passer */
    /*     par les généralités, les comptes, les échéances ... */

    while ( node_1 )
    {
	/* on récupère ici les généralités */

	if ( !strcmp ( node_1 -> name,
		       "Generalites" ) )
	    recuperation_generalites_xml ( node_1 -> children );

	/* on recupère ici les comptes et operations */

	if ( !strcmp ( node_1 -> name,
		       "Comptes" ))
	    recuperation_comptes_xml ( node_1 -> children );

	/* on recupère ici les echeances */

	if ( !strcmp ( node_1 -> name,
		       "Echeances" ))
	    recuperation_echeances_xml ( node_1 -> children );

	/* on recupère ici les tiers */

	if ( !strcmp ( node_1 -> name,
		       "Tiers" ))
	    recuperation_tiers_xml ( node_1 -> children );

	/* on recupère ici les categories */

	if ( !strcmp ( node_1 -> name,
		       "Categories" ))
	    recuperation_categories_xml( node_1 -> children );

	/* on recupère ici les imputations */

	if ( !strcmp ( node_1 -> name,
		       "Imputations" ))
	    recuperation_imputations_xml ( node_1 -> children );

	/* on recupère ici les devises */

	if ( !strcmp ( node_1 -> name,
		       "Devises" ))
	    recuperation_devises_xml ( node_1 -> children );

	/* on recupère ici les banques */

	if ( !strcmp ( node_1 -> name,
		       "Banques" ))
	    recuperation_banques_xml ( node_1 -> children );

	/* on recupère ici les exercices */

	if ( !strcmp ( node_1 -> name,
		       "Exercices" ))
	    recuperation_exercices_xml( node_1 -> children );

	/* on recupère ici les rapprochements */

	if ( !strcmp ( node_1 -> name,
		       "Rapprochements" ))
	    recuperation_rapprochements_xml ( node_1 -> children );

	/* on recupère ici les etats */

	if ( !strcmp ( node_1 -> name,
		       "Etats" ))
	    recuperation_etats_xml ( node_1 -> children );

	node_1 = node_1 -> next;
    }

    etat.en_train_de_charger = 0;

    /* on libère la mémoire */

    xmlFreeDoc ( doc );

    return TRUE;
}
/*****************************************************************************/





/***********************************************************************************************************/
void switch_t_r ( void )
{
/* cette fonction fait le tour des opérations et change le marquage T et R des opés */
/*     R devient pointe=3 */
/*     T devient pointe=2 */

/*     à n'appeler que pour une version antérieure à 0.5.1 */

    gint i;

    if ( !nb_comptes )
	return;
    
    if ( DEBUG )
	printf ( "switch_t_r\n");

    for ( i=0 ; i<nb_comptes ; i++ )
    {	
	GSList *liste_tmp;
	
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{ 
	    struct structure_operation *operation;

	    operation = liste_tmp -> data;

	    switch ( operation -> pointe )
	    {
		case 2 :
		    operation -> pointe = 3;
		    break;
		case 3:
		    operation -> pointe = 2;
		    break;
	    }
	    liste_tmp = liste_tmp -> next;
	}
    }
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* fonction qui charge les généralités dans un fichier xml grisbi */
/* appelée quand la fonction de chargement retouver les généralités */
/***********************************************************************************************************/

gboolean recuperation_generalites_xml ( xmlNodePtr node_generalites )
{
    while ( node_generalites )
    {
	if ( !strcmp ( node_generalites -> name,
		       "Backup" ))
	    nom_fichier_backup = xmlNodeGetContent ( node_generalites );

	if ( !strcmp ( node_generalites -> name,
		       "Titre" ))
	    titre_fichier = xmlNodeGetContent ( node_generalites );

	if ( !strcmp ( node_generalites -> name,
		       "Adresse_commune" ))
	    adresse_commune = xmlNodeGetContent ( node_generalites );

	if ( !strcmp ( node_generalites -> name,
		       "Adresse_secondaire" ))
	    adresse_secondaire = xmlNodeGetContent ( node_generalites );

	if ( !strcmp ( node_generalites -> name,
		       "Numero_devise_totaux_tiers" ))
	    no_devise_totaux_tiers = my_atoi ( xmlNodeGetContent ( node_generalites ));

	if ( !strcmp ( node_generalites -> name,
		       "Type_affichage_des_echeances" ))
	    affichage_echeances = my_atoi ( xmlNodeGetContent ( node_generalites ));

	if ( !strcmp ( node_generalites -> name,
		       "Affichage_echeances_perso_nb_libre" ))
	    affichage_echeances_perso_nb_libre = my_atoi ( xmlNodeGetContent ( node_generalites ));

	if ( !strcmp ( node_generalites -> name,
		       "Type_affichage_perso_echeances" ))
	    affichage_echeances_perso_j_m_a = my_atoi ( xmlNodeGetContent ( node_generalites ));

	if ( !strcmp ( node_generalites -> name,
		       "Numero_derniere_operation" ))
	    no_derniere_operation= my_atoi ( xmlNodeGetContent ( node_generalites ));

	if ( !strcmp ( node_generalites -> name,
		       "Echelle_date_import" ))
	    valeur_echelle_recherche_date_import = my_atoi ( xmlNodeGetContent ( node_generalites ));

	if ( !strcmp ( node_generalites -> name,
		       "Utilise_logo" ))
	    etat.utilise_logo = my_atoi ( xmlNodeGetContent ( node_generalites ));

	if ( !strcmp ( node_generalites -> name,
		       "Chemin_logo" ))
	    chemin_logo = xmlNodeGetContent ( node_generalites );

	if ( !strcmp ( node_generalites -> name,
		       "Caracteristiques_par_compte" ))
	    etat.retient_affichage_par_compte = my_atoi( xmlNodeGetContent (node_generalites ));

	if ( !strcmp ( node_generalites -> name,
		       "Affichage_opes" ))
	{
	    gchar **pointeur_char;
	    gint i, j;

	    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_generalites ),
					 "-",
					 28 );

	    for ( i=0 ; i<4 ; i++ )
		for ( j=0 ; j< 7 ; j++ )
		    tab_affichage_ope[i][j] = my_atoi ( pointeur_char[j + i*7]);

	    g_strfreev ( pointeur_char );
	}

	if ( !strcmp ( node_generalites -> name,
		       "Rapport_largeur_col" ))
	{
	    gchar **pointeur_char;
	    gint i;

	    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_generalites ),
					 "-",
					 7 );

	    for ( i=0 ; i<7 ; i++ )
		rapport_largeur_colonnes[i] = my_atoi ( pointeur_char[i]);

	    g_strfreev ( pointeur_char );
	}

	if ( !strcmp ( node_generalites -> name,
		       "Ligne_aff_une_ligne" ))
	    ligne_affichage_une_ligne = my_atoi ( xmlNodeGetContent ( node_generalites ));

	if ( !strcmp ( node_generalites -> name,
		       "Lignes_aff_deux_lignes" ))
	{
	    gchar **pointeur_char;

	    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_generalites ),
					 "-",
					 2 );

	    lignes_affichage_deux_lignes = NULL;
	    lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
							    GINT_TO_POINTER ( my_atoi ( pointeur_char[0] )));
	    lignes_affichage_deux_lignes = g_slist_append ( lignes_affichage_deux_lignes,
							    GINT_TO_POINTER ( my_atoi ( pointeur_char[1] )));

	    g_strfreev ( pointeur_char );
	}

	if ( !strcmp ( node_generalites -> name,
		       "Lignes_aff_trois_lignes" ))
	{
	    gchar **pointeur_char;

	    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_generalites ),
					 "-",
					 3 );

	    lignes_affichage_trois_lignes = NULL;
	    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							     GINT_TO_POINTER ( my_atoi ( pointeur_char[0] )));
	    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							     GINT_TO_POINTER ( my_atoi ( pointeur_char[1] )));
	    lignes_affichage_trois_lignes = g_slist_append ( lignes_affichage_trois_lignes,
							     GINT_TO_POINTER ( my_atoi ( pointeur_char[2] )));

	    g_strfreev ( pointeur_char );
	}
	
	if ( !strcmp ( node_generalites -> name,
		       "Formulaire_distinct_par_compte" ))
	    etat.formulaire_distinct_par_compte = my_atoi( xmlNodeGetContent (node_generalites ));

	if ( !strcmp ( node_generalites -> name,
		       "Rapport_largeur_col_echeancier" ))
	{
	    gchar **pointeur_char;
	    gint i;

	    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_generalites ),
					 "-",
					 NB_COLS_SCHEDULER );

	    for ( i=0 ; i<NB_COLS_SCHEDULER ; i++ )
		scheduler_col_width[i] = my_atoi ( pointeur_char[i]);

	    g_strfreev ( pointeur_char );
	}

	node_generalites = node_generalites -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* fonction qui charge les comptes dans un fichier xml grisbi */
/***********************************************************************************************************/

gboolean recuperation_comptes_xml ( xmlNodePtr node_comptes )
{
    while ( node_comptes )
    {

	/* on va recuperer ici les generalites des comptes */

	if ( !strcmp ( node_comptes -> name,
		       "Generalites" ))
	{
	    xmlNodePtr node_generalites;

	    node_generalites = node_comptes -> children;

	    while ( node_generalites )
	    {
		/* recupère l'ordre des comptes */

		if ( !strcmp ( node_generalites -> name,
			       "Ordre_des_comptes" ))
		{
		    gchar **pointeur_char;
		    gint i;

		    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_generalites ),
						 "-",
						 0 );

		    i = 0;
		    ordre_comptes = NULL;

		    while ( pointeur_char[i] )
		    {
			ordre_comptes = g_slist_append ( ordre_comptes,
							 GINT_TO_POINTER ( my_atoi ( pointeur_char[i] )));
			i++;
		    }
		    g_strfreev ( pointeur_char );

		    /* calcule le nb de comptes */

		    nb_comptes = g_slist_length ( ordre_comptes );

		    /* Creation du tableau de pointeur vers les structures de comptes */

		    p_tab_nom_de_compte = malloc ( nb_comptes * sizeof ( gpointer ));
		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
		}

		/* recupère le compte courant */

		if ( !strcmp ( node_generalites -> name,
			       "Compte_courant" ))
		{
		    compte_courant = my_atoi ( xmlNodeGetContent ( node_generalites ));
		}

		node_generalites = node_generalites -> next;
	    }
	}

	/* on recupère les details de chaque compte */

	if ( !strcmp ( node_comptes -> name,
		       "Compte" ))
	{
	    xmlNodePtr node_nom_comptes;

	    /* normalement p_tab_nom_de_compte_variable est dejà place */

	    /* on cree la structure du compte */

	    *p_tab_nom_de_compte_variable = calloc ( 1,
						     sizeof (struct donnees_compte));

	    /* on fait le tour dans l'arbre nom, cad : les details, details de type et details des operations */

	    node_nom_comptes = node_comptes -> children;

	    while ( node_nom_comptes )
	    {
		/* on recupère les details du compte */

		if ( !strcmp ( node_nom_comptes -> name,
			       "Details" ))
		{
		    xmlNodePtr node_detail;

		    node_detail = node_nom_comptes -> children;

		    while ( node_detail )
		    {
			if ( node_detail -> type != XML_TEXT_NODE )
			{

			    if ( !strcmp ( node_detail -> name,
					   "Nom" ))
				NOM_DU_COMPTE = xmlNodeGetContent ( node_detail );

			    if ( !strcmp ( node_detail -> name,
					   "Id_compte" ))
			    {
				ID_COMPTE = xmlNodeGetContent ( node_detail );
				if ( !strlen ( ID_COMPTE ))
				    ID_COMPTE = NULL;
			    }
			    if ( !strcmp ( node_detail -> name,
					   "No_de_compte" ))
				NO_COMPTE = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Titulaire" ))
				TITULAIRE = xmlNodeGetContent ( node_detail );

			    if ( !strcmp ( node_detail -> name,
					   "Type_de_compte" ))
				TYPE_DE_COMPTE = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Devise" ))
				DEVISE = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Banque" ))
				BANQUE = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Guichet" ))
				NO_GUICHET = xmlNodeGetContent ( node_detail );

			    if ( !strcmp ( node_detail -> name,
					   "No_compte_banque" ))
				NO_COMPTE_BANQUE = xmlNodeGetContent ( node_detail );

			    if ( !strcmp ( node_detail -> name,
					   "Cle_du_compte" ))
				CLE_COMPTE = xmlNodeGetContent ( node_detail );

			    if ( !strcmp ( node_detail -> name,
					   "Solde_initial" ))
				SOLDE_INIT = my_strtod ( xmlNodeGetContent ( node_detail ),
							 NULL );

			    if ( !strcmp ( node_detail -> name,
					   "Solde_mini_voulu" ))
				SOLDE_MINI_VOULU = my_strtod ( xmlNodeGetContent ( node_detail ),
							       NULL );

			    if ( !strcmp ( node_detail -> name,
					   "Solde_mini_autorise" ))
				SOLDE_MINI = my_strtod ( xmlNodeGetContent ( node_detail ),
							 NULL );

			    if ( !strcmp ( node_detail -> name,
					   "Date_dernier_releve" ))
			    {
				gchar **pointeur_char;

				if ( xmlNodeGetContent ( node_detail ) &&
				     strlen (xmlNodeGetContent (node_detail)) > 0 )
				{
				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail ), "/", 3 );
				    DATE_DERNIER_RELEVE = g_date_new_dmy ( my_atoi ( pointeur_char [0] ),
									   my_atoi ( pointeur_char [1] ),
									   my_atoi ( pointeur_char [2] ));
				    g_strfreev ( pointeur_char );
				}
			    }

			    if ( !strcmp ( node_detail -> name,
					   "Solde_dernier_releve" ))
				SOLDE_DERNIER_RELEVE = my_strtod ( xmlNodeGetContent ( node_detail ),
								   NULL );

			    if ( !strcmp ( node_detail -> name,
					   "Dernier_no_de_rapprochement" ))
				DERNIER_NO_RAPPROCHEMENT = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Compte_cloture" ))
				COMPTE_CLOTURE = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Affichage_r" ))
				AFFICHAGE_R  = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Nb_lignes_ope" ))
				NB_LIGNES_OPE = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Commentaires" ))
				COMMENTAIRE = xmlNodeGetContent ( node_detail );

			    if ( !strcmp ( node_detail -> name,
					   "Adresse_du_titulaire" ))
				ADRESSE_TITULAIRE = xmlNodeGetContent ( node_detail );

			    if ( !strcmp ( node_detail -> name,
					   "Type_defaut_debit" ))
				TYPE_DEFAUT_DEBIT = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Type_defaut_credit" ))
				TYPE_DEFAUT_CREDIT = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Tri_par_type" ))
				TRI = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Neutres_inclus" ))
				NEUTRES_INCLUS = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "Ordre_du_tri" ))
			    {
				LISTE_TRI = NULL;

				if ( xmlNodeGetContent ( node_detail ))
				{
				    gchar **pointeur_char;
				    gint i;

				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail ),
								 "/",
								 0 );

				    i = 0;

				    while ( pointeur_char[i] )
				    {
					LISTE_TRI = g_slist_append ( LISTE_TRI,
								     GINT_TO_POINTER ( my_atoi ( pointeur_char[i] )));
					i++;
				    }
				    g_strfreev ( pointeur_char );
				}
			    }

			    if ( !strcmp ( node_detail -> name,
					   "Classement_croissant" ))
				CLASSEMENT_CROISSANT = my_atoi ( xmlNodeGetContent ( node_detail ));

			    if ( !strcmp ( node_detail -> name,
					   "No_classement" ))
				NO_CLASSEMENT = my_atoi ( xmlNodeGetContent ( node_detail ));

			    /* récupération de l'agencement du formulaire */

			    if ( !strcmp ( node_detail -> name,
					   "Nb_colonnes_formulaire" ))
			    {
				if ( !ORGANISATION_FORMULAIRE )
				    ORGANISATION_FORMULAIRE = calloc ( 1,
								       sizeof ( struct organisation_formulaire ));
				ORGANISATION_FORMULAIRE -> nb_colonnes = my_atoi ( xmlNodeGetContent ( node_detail ));
			    }

			    if ( !strcmp ( node_detail -> name,
					   "Nb_lignes_formulaire" ))
			    {
				if ( !ORGANISATION_FORMULAIRE )
				    ORGANISATION_FORMULAIRE = calloc ( 1,
								       sizeof ( struct organisation_formulaire ));
				ORGANISATION_FORMULAIRE -> nb_lignes = my_atoi ( xmlNodeGetContent ( node_detail ));
			    }


			    if ( !strcmp ( node_detail -> name,
					   "Organisation_formulaire" ))
			    {
				gchar **pointeur_char;
				gint i, j;

				if ( !ORGANISATION_FORMULAIRE )
				    ORGANISATION_FORMULAIRE = calloc ( 1,
								       sizeof ( struct organisation_formulaire ));

				pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail ),
							     "-",
							     0 );

				for ( i=0 ; i<4 ; i++ )
				    for ( j=0 ; j< 6 ; j++ )
					ORGANISATION_FORMULAIRE -> tab_remplissage_formulaire[i][j] = my_atoi ( pointeur_char[j + i*6]);

				g_strfreev ( pointeur_char );
			    }


			    if ( !strcmp ( node_detail -> name,
					   "Largeur_col_formulaire" ))
			    {
				gchar **pointeur_char;
				gint i;

				if ( !ORGANISATION_FORMULAIRE )
				    ORGANISATION_FORMULAIRE = calloc ( 1,
								       sizeof ( struct organisation_formulaire ));

				pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail ),
							     "-",
							     0 );

				for ( i=0 ; i<6 ; i++ )
					ORGANISATION_FORMULAIRE -> taille_colonne_pourcent[i] = my_atoi ( pointeur_char[i]);

				g_strfreev ( pointeur_char );
			    }

			}
			node_detail = node_detail -> next;
		    }
		}

		/* dans certains cas d'import qif, le nom du compte peut être nul */
		/* dans ce cas le met à "" */

		if ( !NOM_DU_COMPTE )
		    NOM_DU_COMPTE = g_strdup ( "" );

		/* on recupère ici le detail des types */

		if ( !strcmp ( node_nom_comptes -> name,
			       "Detail_de_Types" ))
		{
		    xmlNodePtr node_type;

		    node_type = node_nom_comptes -> children;
		    TYPES_OPES = NULL;

		    while ( node_type )
		    {
			struct struct_type_ope *type;

			type = calloc ( 1,
					sizeof ( struct struct_type_ope ));

			if ( node_type -> type != XML_TEXT_NODE )
			{
			    type -> no_type = my_atoi ( xmlGetProp ( node_type,
								     "No" ));
			    type -> nom_type = xmlGetProp ( node_type,
							    "Nom" );
			    type -> signe_type = my_atoi ( xmlGetProp ( node_type,
									"Signe" ));
			    type -> affiche_entree = my_atoi ( xmlGetProp ( node_type,
									    "Affiche_entree" ));
			    type -> numerotation_auto = my_atoi ( xmlGetProp ( node_type,
									       "Numerotation_auto" ));
			    type -> no_en_cours = my_atoi ( xmlGetProp ( node_type,
									 "No_en_cours" ));

			    type -> no_compte = NO_COMPTE;

			    TYPES_OPES = g_slist_append ( TYPES_OPES,
							  type );
			}

			node_type = node_type -> next;
		    }
		}


		/* on recupère ici le detail des opes */

		if ( !strcmp ( node_nom_comptes -> name,
			       "Detail_des_operations" ))
		{
		    xmlNodePtr node_ope;

		    node_ope = node_nom_comptes -> children;
		    LISTE_OPERATIONS = NULL;

		    while ( node_ope )
		    {
			struct structure_operation *operation;
			gchar **pointeur_char;
			gchar *pointeur;

			operation = calloc ( 1,
					     sizeof (struct structure_operation ));

			if ( node_ope -> type != XML_TEXT_NODE )
			{
			    operation -> no_operation = my_atoi ( xmlGetProp ( node_ope, "No" ));

			    operation -> id_operation = xmlGetProp ( node_ope,
								     "Id" );
			    if ( operation -> id_operation &&
				 !strlen ( operation -> id_operation ))
				operation -> id_operation = NULL;

			    pointeur_char = g_strsplit ( xmlGetProp ( node_ope , "D" ), "/", 3 );
			    operation -> jour = my_atoi ( pointeur_char[0] );
			    operation -> mois = my_atoi ( pointeur_char[1] );
			    operation -> annee = my_atoi ( pointeur_char[2] );
			    operation -> date = g_date_new_dmy ( operation -> jour,
								 operation -> mois,
								 operation -> annee );
			    g_strfreev ( pointeur_char );

			    /* GDC prise en compte de la lecture de la date bancaire */

			    pointeur = xmlGetProp ( node_ope,
						    "Db" );

			    if ( pointeur )
			    {
				pointeur_char = g_strsplit ( pointeur,
							     "/",
							     3 );
				operation -> jour_bancaire = my_atoi ( pointeur_char[0] );
				operation -> mois_bancaire = my_atoi ( pointeur_char[1] );
				operation -> annee_bancaire = my_atoi ( pointeur_char[2] );

				if ( operation -> jour_bancaire )
				    operation -> date_bancaire = g_date_new_dmy ( operation -> jour_bancaire,
										  operation -> mois_bancaire,
										  operation -> annee_bancaire );
				else
				    operation -> date_bancaire = NULL;

				g_strfreev ( pointeur_char );
			    }
			    else
			    {
				operation -> jour_bancaire = 0;
				operation -> mois_bancaire = 0;
				operation -> annee_bancaire = 0;
				operation -> date_bancaire = NULL;
			    }

			    /* GDCFin */

			    operation -> montant = my_strtod ( xmlGetProp ( node_ope,
									    "M" ),
							       NULL );

			    operation -> devise = my_atoi ( xmlGetProp ( node_ope,
									 "De" ));

			    operation -> une_devise_compte_egale_x_devise_ope = my_atoi ( xmlGetProp ( node_ope,
												       "Rdc" ));

			    operation -> taux_change = my_strtod ( xmlGetProp ( node_ope,
										"Tc" ),
								   NULL );

			    operation -> frais_change = my_strtod ( xmlGetProp ( node_ope,
										 "Fc" ),
								    NULL );

			    operation -> tiers = my_atoi ( xmlGetProp ( node_ope,
									"T" ));

			    operation -> categorie = my_atoi ( xmlGetProp ( node_ope,
									    "C" ));

			    operation -> sous_categorie = my_atoi ( xmlGetProp ( node_ope,
										 "Sc" ));

			    operation -> operation_ventilee = my_atoi ( xmlGetProp ( node_ope,
										     "Ov" ));

			    operation -> notes = xmlGetProp ( node_ope,
							      "N" );
			    if ( !strlen ( operation -> notes ))
				operation -> notes = NULL;

			    operation -> type_ope = my_atoi ( xmlGetProp ( node_ope,
									   "Ty" ));

			    operation -> contenu_type = xmlGetProp ( node_ope,
								     "Ct" );
			    if ( !strlen ( operation -> contenu_type ))
				operation -> contenu_type = NULL;

			    operation -> pointe = my_atoi ( xmlGetProp ( node_ope,
									 "P" ));

			    operation -> auto_man = my_atoi ( xmlGetProp ( node_ope,
									   "A" ));

			    operation -> no_rapprochement = my_atoi ( xmlGetProp ( node_ope,
										   "R" ));

			    operation -> no_exercice = my_atoi ( xmlGetProp ( node_ope,
									      "E" ));

			    operation -> imputation = my_atoi ( xmlGetProp ( node_ope,
									     "I" ));

			    operation -> sous_imputation = my_atoi ( xmlGetProp ( node_ope,
										  "Si" ));

			    operation -> no_piece_comptable = xmlGetProp ( node_ope,
									   "Pc" );
			    if ( !strlen ( operation -> no_piece_comptable ))
				operation -> no_piece_comptable = NULL;

			    operation -> info_banque_guichet = xmlGetProp ( node_ope,
									    "Ibg" );
			    if ( !strlen ( operation -> info_banque_guichet ))
				operation -> info_banque_guichet = NULL;

			    operation -> relation_no_operation = my_atoi ( xmlGetProp ( node_ope,
											"Ro" ));

			    operation -> relation_no_compte = my_atoi ( xmlGetProp ( node_ope,
										     "Rc" ));

			    operation -> no_operation_ventilee_associee = my_atoi ( xmlGetProp ( node_ope,
												 "Va" ));


			    /* on met le compte associe */

			    operation -> no_compte = NO_COMPTE;

			    LISTE_OPERATIONS = g_slist_append ( LISTE_OPERATIONS,
								operation);
			}

			node_ope = node_ope -> next;
		    }
		}
		node_nom_comptes = node_nom_comptes -> next;
	    }
	    /* 		    le compte est fini, on peut mettre à jour qques variables */


	    if ( SOLDE_COURANT < SOLDE_MINI_VOULU )
		MESSAGE_SOUS_MINI_VOULU = 0;
	    else
		MESSAGE_SOUS_MINI_VOULU = 1;

	    if ( SOLDE_COURANT < SOLDE_MINI )
		MESSAGE_SOUS_MINI = 0;
	    else
		MESSAGE_SOUS_MINI = 1;

	    /*       la selection au depart est en bas de la liste */

	    VALUE_AJUSTEMENT_LISTE_OPERATIONS = -1;
	    OPERATION_SELECTIONNEE = GINT_TO_POINTER (-1);


	    /* on incremente p_tab_nom_de_compte_variable pour le compte suivant */

	    p_tab_nom_de_compte_variable++;
	}

	node_comptes = node_comptes -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* fonction qui charge les échéances dans un fichier xml grisbi */
/***********************************************************************************************************/

gboolean recuperation_echeances_xml ( xmlNodePtr node_echeances )
{
    while ( node_echeances )
    {
	/* on va recuperer ici les generalites des echeances */

	if ( !strcmp ( node_echeances -> name,
		       "Generalites" ))
	{
	    xmlNodePtr node_generalites;

	    node_generalites = node_echeances -> children;

	    while ( node_generalites )
	    {
		if ( !strcmp ( node_generalites -> name,
			       "Nb_echeances" ))
		    nb_echeances = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "No_derniere_echeance" ))
		    no_derniere_echeance = my_atoi ( xmlNodeGetContent ( node_generalites ));

		node_generalites = node_generalites -> next;
	    }
	}


	/* on va recuperer ici les echeances */

	if ( !strcmp ( node_echeances -> name,
		       "Detail_des_echeances" ))
	{
	    xmlNodePtr node_detail;

	    liste_struct_echeances = NULL;

	    node_detail = node_echeances -> children;

	    while ( node_detail )
	    {
		struct operation_echeance *operation_echeance;
		gchar **pointeur_char;

		operation_echeance = calloc ( 1,
					      sizeof (struct operation_echeance ));

		if ( node_detail -> type != XML_TEXT_NODE )
		{
		    operation_echeance -> no_operation = my_atoi ( xmlGetProp ( node_detail,
										"No" ));

		    pointeur_char = g_strsplit ( xmlGetProp ( node_detail ,
							      "Date" ),
						 "/",
						 3 );
		    operation_echeance -> jour = my_atoi ( pointeur_char[0] );
		    operation_echeance -> mois = my_atoi ( pointeur_char[1] );
		    operation_echeance -> annee = my_atoi ( pointeur_char[2] );
		    operation_echeance -> date = g_date_new_dmy ( operation_echeance -> jour,
								  operation_echeance -> mois,
								  operation_echeance -> annee );
		    g_strfreev ( pointeur_char );

		    operation_echeance -> compte = my_atoi ( xmlGetProp ( node_detail,
									  "Compte" ));

		    operation_echeance -> montant = my_strtod ( xmlGetProp ( node_detail,
									     "Montant" ),
								NULL );

		    operation_echeance -> devise = my_atoi ( xmlGetProp ( node_detail,
									  "Devise" ));

		    operation_echeance -> tiers = my_atoi ( xmlGetProp ( node_detail,
									 "Tiers" ));

		    operation_echeance -> categorie = my_atoi ( xmlGetProp ( node_detail,
									     "Categorie" ));

		    operation_echeance -> sous_categorie = my_atoi ( xmlGetProp ( node_detail,
										  "Sous-categorie" ));

		    operation_echeance -> compte_virement = my_atoi ( xmlGetProp ( node_detail,
										   "Virement_compte" ));

		    operation_echeance -> type_ope = my_atoi ( xmlGetProp ( node_detail,
									    "Type" ));

		    operation_echeance -> type_contre_ope = my_atoi ( xmlGetProp ( node_detail,
										   "Type_contre_ope" ));

		    operation_echeance -> contenu_type = xmlGetProp ( node_detail,
								      "Contenu_du_type" );
		    if ( !strlen ( operation_echeance -> contenu_type ))
			operation_echeance -> contenu_type = NULL;

		    operation_echeance -> no_exercice = my_atoi ( xmlGetProp ( node_detail,
									       "Exercice" ));

		    operation_echeance -> imputation = my_atoi ( xmlGetProp ( node_detail,
									      "Imputation" ));

		    operation_echeance -> sous_imputation = my_atoi ( xmlGetProp ( node_detail,
										   "Sous-imputation" ));

		    operation_echeance -> notes = xmlGetProp ( node_detail,
							       "Notes" );
		    if ( !strlen ( operation_echeance -> notes ))
			operation_echeance -> notes = NULL;

		    operation_echeance -> auto_man = my_atoi ( xmlGetProp ( node_detail,
									    "Automatique" ));

		    operation_echeance -> periodicite = my_atoi ( xmlGetProp ( node_detail,
									       "Periodicite" ));

		    operation_echeance -> intervalle_periodicite_personnalisee = my_atoi ( xmlGetProp ( node_detail,
													"Intervalle_periodicite" ));

		    operation_echeance -> periodicite_personnalisee = my_atoi ( xmlGetProp ( node_detail,
											     "Periodicite_personnalisee" ));

		    if ( strlen ( xmlGetProp ( node_detail ,
					       "Date_limite" )))
		    {
			pointeur_char = g_strsplit ( xmlGetProp ( node_detail ,
								  "Date_limite" ),
						     "/",
						     3 );

			operation_echeance -> jour_limite = my_atoi ( pointeur_char[0] );
			operation_echeance -> mois_limite = my_atoi ( pointeur_char[1] );
			operation_echeance -> annee_limite = my_atoi ( pointeur_char[2] );
			operation_echeance -> date_limite = g_date_new_dmy ( operation_echeance -> jour_limite,
									     operation_echeance -> mois_limite,
									     operation_echeance -> annee_limite );
			g_strfreev ( pointeur_char );
		    }
		    else
		    {
			operation_echeance -> jour_limite = 0;
			operation_echeance -> mois_limite = 0;
			operation_echeance -> annee_limite = 0;
			operation_echeance -> date_limite = NULL;
		    }

		    operation_echeance -> operation_ventilee = my_atoi ( xmlGetProp ( node_detail,
										      "Ech_ventilee" ));

		    operation_echeance -> no_operation_ventilee_associee = my_atoi ( xmlGetProp ( node_detail,
												  "No_ech_associee" ));



		    liste_struct_echeances = g_slist_append ( liste_struct_echeances,
							      operation_echeance);
		}

		node_detail = node_detail -> next;
	    }
	}
	node_echeances = node_echeances -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* fonction qui charge les tiers dans un fichier xml grisbi */
/***********************************************************************************************************/
gboolean recuperation_tiers_xml ( xmlNodePtr node_tiers )
{
    while ( node_tiers )
    {
	/* on va recuperer ici les generalites des tiers */

	if ( !strcmp ( node_tiers -> name,
		       "Generalites" ))
	{
	    xmlNodePtr node_generalites;

	    node_generalites = node_tiers -> children;

	    while ( node_generalites )
	    {
		if ( !strcmp ( node_generalites -> name,
			       "Nb_tiers" ))
		    nb_enregistrements_tiers = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "No_dernier_tiers" ))
		    no_dernier_tiers = my_atoi ( xmlNodeGetContent ( node_generalites ));


		node_generalites = node_generalites -> next;
	    }
	}


	/* on va recuperer ici les tiers */

	if ( !strcmp ( node_tiers -> name,
		       "Detail_des_tiers" ))
	{
	    xmlNodePtr node_detail;

	    liste_struct_tiers = NULL;
	    node_detail = node_tiers -> children;

	    while ( node_detail )
	    {
		struct struct_tiers *tiers;

		tiers = calloc ( 1,
				 sizeof ( struct struct_tiers ) );

		if ( node_detail -> type != XML_TEXT_NODE )
		{
		    tiers -> no_tiers = my_atoi ( xmlGetProp ( node_detail,
							       "No" ));
		    tiers -> nom_tiers = xmlGetProp ( node_detail,
						      "Nom" );
		    tiers -> texte = xmlGetProp ( node_detail,
						  "Informations" );
		    if ( !strlen ( tiers -> texte ))
			tiers -> texte = NULL;

		    tiers -> liaison = my_atoi ( xmlGetProp ( node_detail,
							      "Liaison" ));

		    liste_struct_tiers = g_slist_append ( liste_struct_tiers,
							  tiers );
		}

		node_detail = node_detail -> next;
	    }
	}
	node_tiers = node_tiers -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* fonction qui charge les categ dans un fichier xml grisbi */
/***********************************************************************************************************/
gboolean recuperation_categories_xml ( xmlNodePtr node_categories )
{
    while ( node_categories )
    {
	/* on va recuperer ici les generalites des categories */

	if ( !strcmp ( node_categories -> name,
		       "Generalites" ))
	{
	    xmlNodePtr node_generalites;

	    node_generalites = node_categories -> children;

	    while ( node_generalites )
	    {
		if ( !strcmp ( node_generalites -> name,
			       "Nb_categories" ))
		    nb_enregistrements_categories = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "No_derniere_categorie" ))
		    no_derniere_categorie = my_atoi ( xmlNodeGetContent ( node_generalites ));

		node_generalites = node_generalites -> next;
	    }
	}


	/* on va recuperer ici les categories */

	if ( !strcmp ( node_categories -> name,
		       "Detail_des_categories" ))
	{
	    xmlNodePtr node_detail;

	    liste_struct_categories = NULL;
	    node_detail = node_categories -> children;

	    while ( node_detail )
	    {
		struct struct_categ *categorie;
		xmlNodePtr node_sous_categ;

		categorie = calloc ( 1,
				     sizeof ( struct struct_categ ) );

		if ( node_detail -> type != XML_TEXT_NODE )
		{
		    categorie -> no_categ = my_atoi ( xmlGetProp ( node_detail,
								   "No" ));
		    categorie -> nom_categ = xmlGetProp ( node_detail,
							  "Nom" );
		    categorie -> type_categ = my_atoi ( xmlGetProp ( node_detail,
								     "Type" ));
		    categorie -> no_derniere_sous_categ = my_atoi ( xmlGetProp ( node_detail,
										 "No_derniere_sous_cagegorie" ));

		    /*  pour chaque categorie, on recupère les sous-categories */

		    categorie -> liste_sous_categ = NULL;
		    node_sous_categ = node_detail -> children;

		    while ( node_sous_categ )
		    {
			struct struct_sous_categ *sous_categ;

			if ( node_sous_categ -> type != XML_TEXT_NODE )
			{
			    sous_categ = calloc ( 1, sizeof ( struct struct_sous_categ ) );

			    sous_categ -> no_sous_categ = my_atoi ( xmlGetProp ( node_sous_categ,
										 "No" ));
			    sous_categ -> nom_sous_categ = xmlGetProp ( node_sous_categ,
									"Nom" );

			    categorie -> liste_sous_categ = g_slist_append ( categorie -> liste_sous_categ,
									     sous_categ );
			}
			node_sous_categ = node_sous_categ -> next;
		    }

		    liste_struct_categories = g_slist_append ( liste_struct_categories,
							       categorie );
		}

		node_detail = node_detail -> next;
	    }
	}
	node_categories = node_categories -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/





/***********************************************************************************************************/
/* fonction qui charge les ib dans un fichier xml grisbi */
/***********************************************************************************************************/
gboolean recuperation_imputations_xml ( xmlNodePtr node_imputations )
{
    while ( node_imputations )
    {
	/* on va recuperer ici les generalites des imputations */

	if ( !strcmp ( node_imputations -> name,
		       "Generalites" ))
	{
	    xmlNodePtr node_generalites;

	    node_generalites = node_imputations -> children;

	    while ( node_generalites )
	    {
		if ( !strcmp ( node_generalites -> name,
			       "Nb_imputations" ))
		    nb_enregistrements_imputations = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "No_derniere_imputation" ))
		    no_derniere_imputation = my_atoi ( xmlNodeGetContent ( node_generalites ));

		node_generalites = node_generalites -> next;
	    }
	}


	/* on va recuperer ici les imputations */

	if ( !strcmp ( node_imputations -> name,
		       "Detail_des_imputations" ))
	{
	    xmlNodePtr node_detail;

	    liste_struct_imputation = NULL;
	    node_detail = node_imputations -> children;

	    while ( node_detail )
	    {
		struct struct_imputation *imputation;
		xmlNodePtr node_sous_imputation;

		imputation = calloc ( 1,
				      sizeof ( struct struct_imputation ) );

		if ( node_detail -> type != XML_TEXT_NODE )
		{
		    imputation -> no_imputation = my_atoi ( xmlGetProp ( node_detail,
									 "No" ));
		    imputation -> nom_imputation = xmlGetProp ( node_detail,
								"Nom" );
		    imputation -> type_imputation = my_atoi ( xmlGetProp ( node_detail,
									   "Type" ));
		    imputation -> no_derniere_sous_imputation = my_atoi ( xmlGetProp ( node_detail,
										       "No_derniere_sous_imputation" ));

		    /*  pour chaque categorie, on recupère les sous-categories */

		    imputation -> liste_sous_imputation = NULL;
		    node_sous_imputation = node_detail -> children;

		    while ( node_sous_imputation )
		    {
			struct struct_sous_imputation *sous_imputation;

			if ( node_sous_imputation -> type != XML_TEXT_NODE )
			{
			    sous_imputation = calloc ( 1, sizeof ( struct struct_sous_imputation ) );

			    sous_imputation -> no_sous_imputation = my_atoi ( xmlGetProp ( node_sous_imputation,
											   "No" ));
			    sous_imputation -> nom_sous_imputation = xmlGetProp ( node_sous_imputation,
										  "Nom" );

			    imputation -> liste_sous_imputation = g_slist_append ( imputation -> liste_sous_imputation,
										   sous_imputation );
			}
			node_sous_imputation = node_sous_imputation -> next;
		    }

		    liste_struct_imputation = g_slist_append ( liste_struct_imputation,
							       imputation );
		}

		node_detail = node_detail -> next;
	    }
	}
	node_imputations = node_imputations -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* fonction qui charge les devises d'un fichier xml grisbi */
/***********************************************************************************************************/
gboolean recuperation_devises_xml ( xmlNodePtr node_devises )
{    
    while ( node_devises )
    {
	/* on va recuperer ici les generalites des devises */

	if ( !strcmp ( node_devises -> name,
		       "Generalites" ))
	{
	    xmlNodePtr node_generalites;

	    node_generalites = node_devises -> children;

	    while ( node_generalites )
	    {
		if ( node_generalites -> type != XML_TEXT_NODE )
		{
		    if ( !strcmp ( node_generalites -> name,
				   "Nb_devises" ))
			nb_devises = my_atoi ( xmlNodeGetContent ( node_generalites ));

		    if ( !strcmp ( node_generalites -> name,
				   "No_derniere_devise" ))
			no_derniere_devise = my_atoi ( xmlNodeGetContent ( node_generalites ));
		}

		node_generalites = node_generalites -> next;
	    }
	}


	/* on va recuperer ici les devises */

	if ( !strcmp ( node_devises -> name,
		       "Detail_des_devises" ))
	{
	    xmlNodePtr node_detail;

	    liste_struct_devises = NULL;
	    node_detail = node_devises -> children;

	    while ( node_detail )
	    {
		struct struct_devise *devise;

		devise = calloc ( 1,
				  sizeof ( struct struct_devise ));

		if ( node_detail -> type != XML_TEXT_NODE )
		{

		    devise -> no_devise = my_atoi ( xmlGetProp ( node_detail,
								 "No" ));
		    devise -> nom_devise = xmlGetProp ( node_detail,
							"Nom" );
		    devise -> code_iso4217_devise = xmlGetProp ( node_detail,
								 "IsoCode" );
		    devise -> code_devise = xmlGetProp ( node_detail,
							 "Code" );
		    if ( !strlen ( devise -> code_devise ))
			devise -> code_devise = NULL;
		    if (! devise -> code_iso4217_devise ||
			!strlen ( devise -> code_iso4217_devise ))
			devise -> code_iso4217_devise = NULL;

		    /* 	   la suite n'est utile que pour les anciennes devises qui sont passées à l'euro */
		    /* 	non utilisées pour les autres */

		    devise -> passage_euro = my_atoi ( xmlGetProp ( node_detail,
								    "Passage_euro" ));

		    if ( strlen ( xmlGetProp ( node_detail,
					       "Date_dernier_change" )))
		    {
			gchar **pointeur_char;

			pointeur_char = g_strsplit ( xmlGetProp ( node_detail,
								  "Date_dernier_change" ),
						     "/",
						     3 );

			devise -> date_dernier_change = g_date_new_dmy ( my_atoi ( pointeur_char[0] ),
									 my_atoi ( pointeur_char[1] ),
									 my_atoi ( pointeur_char[2] ));
			g_strfreev ( pointeur_char );
		    }
		    else
			devise -> date_dernier_change = NULL;

		    devise -> une_devise_1_egale_x_devise_2 = my_atoi ( xmlGetProp ( node_detail,
										     "Rapport_entre_devises" ));
		    devise -> no_devise_en_rapport = my_atoi ( xmlGetProp ( node_detail,
									    "Devise_en_rapport" ));
		    devise -> change = my_strtod ( xmlGetProp ( node_detail,
								"Change" ),
						   NULL );

		    liste_struct_devises = g_slist_append ( liste_struct_devises,
							    devise );
		}

		node_detail = node_detail -> next;
	    }
	}
	node_devises = node_devises -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* fonction qui charge les banques d'un fichier xml grisbi */
/***********************************************************************************************************/
gboolean recuperation_banques_xml ( xmlNodePtr node_banques )
{    
    while ( node_banques )
    {
	/* on va recuperer ici les generalites des banques */

	if ( !strcmp ( node_banques -> name,
		       "Generalites" ))
	{
	    xmlNodePtr node_generalites;

	    node_generalites = node_banques -> children;

	    while ( node_generalites )
	    {
		if ( !strcmp ( node_generalites -> name,
			       "Nb_banques" ))
		    nb_banques = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "No_derniere_banque" ))
		    no_derniere_banque = my_atoi ( xmlNodeGetContent ( node_generalites ));


		node_generalites = node_generalites -> next;
	    }
	}


	/* on va recuperer ici les banques */

	if ( !strcmp ( node_banques -> name,
		       "Detail_des_banques" ))
	{
	    xmlNodePtr node_detail;

	    liste_struct_banques = NULL;
	    node_detail = node_banques -> children;

	    while ( node_detail )
	    {
		struct struct_banque *banque;

		banque = calloc ( 1,
				  sizeof ( struct struct_banque ));

		if ( node_detail -> type != XML_TEXT_NODE )
		{
		    banque -> no_banque = my_atoi ( xmlGetProp ( node_detail,
								 "No" ));
		    banque -> nom_banque = xmlGetProp ( node_detail,
							"Nom" );
		    banque -> code_banque = xmlGetProp ( node_detail,
							 "Code" );
		    if ( !strlen ( banque -> code_banque ))
			banque -> code_banque = NULL;
		    banque -> adr_banque = xmlGetProp ( node_detail,
							"Adresse" );
		    if ( !strlen ( banque -> adr_banque ))
			banque -> adr_banque = NULL;
		    banque -> tel_banque = xmlGetProp ( node_detail,
							"Tel" );
		    if ( !strlen ( banque -> tel_banque ))
			banque -> tel_banque = NULL;
		    banque -> email_banque = xmlGetProp ( node_detail,
							  "Mail" );
		    if ( !strlen ( banque -> email_banque ))
			banque -> email_banque = NULL;
		    banque -> web_banque = xmlGetProp ( node_detail,
							"Web" );
		    if ( !strlen ( banque -> web_banque ))
			banque -> web_banque = NULL;
		    banque -> nom_correspondant = xmlGetProp ( node_detail,
							       "Nom_correspondant" );
		    if ( !strlen ( banque -> nom_correspondant ))
			banque -> nom_correspondant = NULL;
		    banque -> fax_correspondant = xmlGetProp ( node_detail,
							       "Fax_correspondant" );
		    if ( !strlen ( banque -> fax_correspondant ))
			banque -> fax_correspondant = NULL;
		    banque -> tel_correspondant = xmlGetProp ( node_detail,
							       "Tel_correspondant" );
		    if ( !strlen ( banque -> tel_correspondant ))
			banque -> tel_correspondant = NULL;
		    banque -> email_correspondant = xmlGetProp ( node_detail,
								 "Mail_correspondant" );
		    if ( !strlen ( banque -> email_correspondant ))
			banque -> email_correspondant = NULL;
		    banque -> remarque_banque = xmlGetProp ( node_detail,
							     "Remarques" );
		    if ( !strlen ( banque -> remarque_banque ))
			banque -> remarque_banque = NULL;

		    liste_struct_banques = g_slist_append ( liste_struct_banques,
							    banque );
		}

		node_detail = node_detail -> next;
	    }
	}
	node_banques = node_banques -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* fonction qui charge les exercices d'un fichier xml grisbi */
/***********************************************************************************************************/
gboolean recuperation_exercices_xml ( xmlNodePtr node_exercices )
{    
    while ( node_exercices )
    {
	/* on va recuperer ici les generalites des exercices */

	if ( !strcmp ( node_exercices -> name,
		       "Generalites" ))
	{
	    xmlNodePtr node_generalites;

	    node_generalites = node_exercices -> children;

	    while ( node_generalites )
	    {
		if ( !strcmp ( node_generalites -> name,
			       "Nb_exercices" ))
		    nb_exercices = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "No_dernier_exercice" ))
		    no_derniere_exercice = my_atoi ( xmlNodeGetContent ( node_generalites ));

		node_generalites = node_generalites -> next;
	    }
	}


	/* on va recuperer ici les exercices */

	if ( !strcmp ( node_exercices -> name,
		       "Detail_des_exercices" ))
	{
	    xmlNodePtr node_detail;

	    liste_struct_exercices = NULL;
	    node_detail = node_exercices -> children;

	    while ( node_detail )
	    {
		struct struct_exercice *exercice;

		exercice = calloc ( 1,
				    sizeof ( struct struct_exercice ));

		if ( node_detail -> type != XML_TEXT_NODE )
		{
		    exercice -> no_exercice = my_atoi ( xmlGetProp ( node_detail,
								     "No" ));
		    exercice -> nom_exercice = xmlGetProp ( node_detail,
							    "Nom" );

		    if ( strlen ( xmlGetProp ( node_detail,
					       "Date_debut" )))
		    {
			gchar **pointeur_char;

			pointeur_char = g_strsplit ( xmlGetProp ( node_detail,
								  "Date_debut" ),
						     "/",
						     3 );

			exercice -> date_debut = g_date_new_dmy ( my_atoi ( pointeur_char[0] ),
								  my_atoi ( pointeur_char[1] ),
								  my_atoi ( pointeur_char[2] ));
			g_strfreev ( pointeur_char );
		    }
		    else
			exercice -> date_debut = NULL;

		    if ( strlen ( xmlGetProp ( node_detail,
					       "Date_fin" )))
		    {
			gchar **pointeur_char;

			pointeur_char = g_strsplit ( xmlGetProp ( node_detail,
								  "Date_fin" ),
						     "/",
						     3 );

			exercice -> date_fin = g_date_new_dmy ( my_atoi ( pointeur_char[0] ),
								my_atoi ( pointeur_char[1] ),
								my_atoi ( pointeur_char[2] ));
			g_strfreev ( pointeur_char );
		    }
		    else
			exercice -> date_fin = NULL;

		    exercice -> affiche_dans_formulaire = my_atoi ( xmlGetProp ( node_detail,
										 "Affiche" ));

		    liste_struct_exercices = g_slist_append ( liste_struct_exercices,
							      exercice );
		}

		node_detail = node_detail -> next;
	    }
	}
	node_exercices = node_exercices -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* fonction qui charge les rapprochements d'un fichier xml grisbi */
/***********************************************************************************************************/
gboolean recuperation_rapprochements_xml ( xmlNodePtr node_rapprochements )
{    
    while ( node_rapprochements )
    {
	/* il n'y a pas de generalites ... */

	/* on va recuperer ici les rapprochements */

	if ( !strcmp ( node_rapprochements -> name,
		       "Detail_des_rapprochements" ))
	{
	    xmlNodePtr node_detail;

	    liste_struct_rapprochements = NULL;
	    node_detail = node_rapprochements -> children;

	    while ( node_detail )
	    {
		struct struct_no_rapprochement *rapprochement;

		rapprochement = calloc ( 1,
					 sizeof ( struct struct_no_rapprochement ));

		if ( node_detail -> type != XML_TEXT_NODE )
		{
		    rapprochement -> no_rapprochement = my_atoi ( xmlGetProp ( node_detail,
									       "No" ));
		    rapprochement -> nom_rapprochement = xmlGetProp ( node_detail,
								      "Nom" );

		    rapprochement -> nom_rapprochement = g_strstrip ( rapprochement -> nom_rapprochement);

		    liste_struct_rapprochements = g_slist_append ( liste_struct_rapprochements,
								   rapprochement );
		}
		node_detail = node_detail -> next;
	    }
	}
	node_rapprochements = node_rapprochements -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/


/***********************************************************************************************************/
/* fonction qui charge les etats d'un fichier xml grisbi */
/***********************************************************************************************************/
gboolean recuperation_etats_xml ( xmlNodePtr node_etats )
{    
    while ( node_etats )
    {
	/* on va recuperer ici les generalites des etats */

	if ( node_etats -> type != XML_TEXT_NODE )
	{

	    if ( !strcmp ( node_etats -> name,
			   "Generalites" ))
	    {
		xmlNodePtr node_generalites;

		node_generalites = node_etats -> children;

		while ( node_generalites )
		{
		    if ( !strcmp ( node_generalites -> name,
				   "No_dernier_etat" ))
			no_dernier_etat = my_atoi ( xmlNodeGetContent ( node_generalites ));

		    node_generalites = node_generalites -> next;
		}
	    }


	    /* on va recuperer ici les etats */

	    if ( !strcmp ( node_etats -> name,
			   "Detail_des_etats" ))
	    {
		xmlNodePtr node_detail;

		liste_struct_etats = NULL;
		node_detail = node_etats -> children;

		/* on fait maintenant le tour de tous les états */

		while ( node_detail )
		{
		    struct struct_etat *etat;
		    xmlNodePtr node_detail_etat;

		    /* création du nouvel état */

		    if ( node_detail -> type != XML_TEXT_NODE &&
			 !strcmp ( node_detail -> name, "Etat") )
		    {
			etat = calloc ( 1, sizeof ( struct struct_etat ));
			node_detail_etat = node_detail -> children;

			/* on récupère les données de l'état */

			while ( node_detail_etat )
			{
			    if ( node_detail_etat -> type != XML_TEXT_NODE )
			    {
				if ( !strcmp ( node_detail_etat -> name,
					       "No" ))
				    etat -> no_etat = my_atoi ( xmlNodeGetContent ( node_detail_etat ));


				if ( !strcmp ( node_detail_etat -> name,
					       "Nom" ))
				    etat -> nom_etat = xmlNodeGetContent ( node_detail_etat );


				if ( !strcmp ( node_detail_etat -> name,
					       "Type_classement" )
				     &&
				     xmlNodeGetContent ( node_detail_etat ))
				{
				    gchar **pointeur_char;
				    gint i;

				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
								 "/",
								 0 );
				    i=0;

				    while ( pointeur_char[i] )
				    {
					etat -> type_classement = g_list_append ( etat -> type_classement,
										  GINT_TO_POINTER ( my_atoi ( pointeur_char[i] )));
					i++;
				    }
				    g_strfreev ( pointeur_char );
				}


				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_r" ))
				    etat -> afficher_r = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_ope" ))
				    etat -> afficher_opes = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_nb_ope" ))
				    etat -> afficher_nb_opes = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_no_ope" ))
				    etat -> afficher_no_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_date_ope" ))
				    etat -> afficher_date_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_tiers_ope" ))
				    etat -> afficher_tiers_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_categ_ope" ))
				    etat -> afficher_categ_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_ss_categ_ope" ))
				    etat -> afficher_sous_categ_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_type_ope" ))
				    etat -> afficher_type_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_ib_ope" ))
				    etat -> afficher_ib_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_ss_ib_ope" ))
				    etat ->afficher_sous_ib_ope  = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_cheque_ope" ))
				    etat -> afficher_cheque_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_notes_ope" ))
				    etat -> afficher_notes_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_pc_ope" ))
				    etat -> afficher_pc_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_rappr_ope" ))
				    etat -> afficher_rappr_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_infobd_ope" ))
				    etat -> afficher_infobd_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_exo_ope" ))
				    etat -> afficher_exo_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Class_ope" ))
				    etat -> type_classement_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_titres_col" ))
				    etat -> afficher_titre_colonnes = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_titres_chgt" ))
				    etat -> type_affichage_titres = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Pas_detail_ventil" ))
				    etat -> pas_detailler_ventilation = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Sep_rev_dep" ))
				    etat -> separer_revenus_depenses = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Devise_gen" ))
				    etat -> devise_de_calcul_general = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Incl_tiers" ))
				    etat -> inclure_dans_tiers = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Ope_click" ))
				    etat -> ope_clickables = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Exo_date" ))
				    etat -> exo_date = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Detail_exo" ))
				    etat -> utilise_detail_exo = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "No_exo" )
				     &&
				     xmlNodeGetContent ( node_detail_etat ))
				{
				    gchar **pointeur_char;
				    gint i;

				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
								 "/",
								 0 );
				    i=0;

				    while ( pointeur_char[i] )
				    {
					etat -> no_exercices = g_slist_append ( etat -> no_exercices,
										GINT_TO_POINTER ( my_atoi ( pointeur_char[i] )));
					i++;
				    }
				    g_strfreev ( pointeur_char );
				}

				if ( !strcmp ( node_detail_etat -> name,
					       "Plage_date" ))
				    etat -> no_plage_date = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Date_debut" ) &&
				     xmlNodeGetContent ( node_detail_etat ) &&
				     (strlen(xmlNodeGetContent(node_detail_etat)) > 0 ))
				{
				    gchar **pointeur_char;

				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
								 "/",
								 3 );

				    etat -> date_perso_debut = g_date_new_dmy ( my_atoi ( pointeur_char[0] ),
										my_atoi ( pointeur_char[1] ),
										my_atoi ( pointeur_char[2] ));
				    g_strfreev ( pointeur_char );
				}

				if ( !strcmp ( node_detail_etat -> name, "Date_fin" ) &&
				     xmlNodeGetContent ( node_detail_etat ) &&
				     (strlen(xmlNodeGetContent(node_detail_etat)) > 0 ))
				{
				    gchar **pointeur_char;

				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
								 "/",
								 3 );

				    etat -> date_perso_fin = g_date_new_dmy ( my_atoi ( pointeur_char[0] ),
									      my_atoi ( pointeur_char[1] ),
									      my_atoi ( pointeur_char[2] ));
				    g_strfreev ( pointeur_char );
				}

				if ( !strcmp ( node_detail_etat -> name,
					       "Utilise_plages" ))
				    etat -> separation_par_plage = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Sep_plages" ))
				    etat -> type_separation_plage = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Sep_exo" ))
				    etat -> separation_par_exo = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Deb_sem_plages" ))
				    etat -> jour_debut_semaine = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Detail_comptes" ))
				    etat -> utilise_detail_comptes = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "No_comptes" )
				     &&
				     xmlNodeGetContent ( node_detail_etat ))
				{
				    gchar **pointeur_char;
				    gint i;

				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
								 "/",
								 0 );
				    i=0;

				    while ( pointeur_char[i] )
				    {
					etat ->no_comptes  = g_slist_append ( etat -> no_comptes,
									      GINT_TO_POINTER ( my_atoi ( pointeur_char[i] )));
					i++;
				    }
				    g_strfreev ( pointeur_char );
				}


				if ( !strcmp ( node_detail_etat -> name,
					       "Grp_ope_compte" ))
				    etat -> regroupe_ope_par_compte = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Total_compte" ))
				    etat -> affiche_sous_total_compte = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_nom_compte" ))
				    etat -> afficher_nom_compte = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Type_vir" ))
				    etat -> type_virement = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "No_comptes_virements" )
				     &&
				     xmlNodeGetContent ( node_detail_etat ))
				{
				    gchar **pointeur_char;
				    gint i;

				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
								 "/",
								 0 );
				    i=0;

				    while ( pointeur_char[i] )
				    {
					etat ->no_comptes_virements  = g_slist_append ( etat -> no_comptes_virements,
											GINT_TO_POINTER ( my_atoi ( pointeur_char[i] )));
					i++;
				    }
				    g_strfreev ( pointeur_char );
				}

				if ( !strcmp ( node_detail_etat -> name,
					       "Exclure_non_vir" ))
				    etat -> exclure_ope_non_virement = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Categ" ))
				    etat -> utilise_categ = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Detail_categ" ))
				    etat -> utilise_detail_categ = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "No_categ" )
				     &&
				     xmlNodeGetContent ( node_detail_etat ))
				{
				    gchar **pointeur_char;
				    gint i;

				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
								 "/",
								 0 );
				    i=0;

				    while ( pointeur_char[i] )
				    {
					etat -> no_categ = g_slist_append ( etat -> no_categ,
									    GINT_TO_POINTER ( my_atoi ( pointeur_char[i] )));
					i++;
				    }
				    g_strfreev ( pointeur_char );
				}

				if ( !strcmp ( node_detail_etat -> name,
					       "Exclut_categ" ))
				    etat -> exclure_ope_sans_categ = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Total_categ" ))
				    etat -> affiche_sous_total_categ = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_ss_categ" ))
				    etat -> afficher_sous_categ = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_pas_ss_categ" ))
				    etat -> afficher_pas_de_sous_categ = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Total_ss_categ" ))
				    etat -> affiche_sous_total_sous_categ = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Devise_categ" ))
				    etat -> devise_de_calcul_categ = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_nom_categ" ))
				    etat -> afficher_nom_categ = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "IB" ))
				    etat -> utilise_ib = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Detail_ib" ))
				    etat -> utilise_detail_ib = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "No_ib" )
				     &&
				     xmlNodeGetContent ( node_detail_etat ))
				{
				    gchar **pointeur_char;
				    gint i;

				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
								 "/",
								 0 );
				    i=0;

				    while ( pointeur_char[i] )
				    {
					etat -> no_ib = g_slist_append ( etat -> no_ib,
									 GINT_TO_POINTER ( my_atoi ( pointeur_char[i] )));
					i++;
				    }
				    g_strfreev ( pointeur_char );
				}


				if ( !strcmp ( node_detail_etat -> name,
					       "Exclut_ib" ))
				    etat -> exclure_ope_sans_ib = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Total_ib" ))
				    etat -> affiche_sous_total_ib = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_ss_ib" ))
				    etat -> afficher_sous_ib = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_pas_ss_ib" ))
				    etat -> afficher_pas_de_sous_ib = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Total_ss_ib" ))
				    etat -> affiche_sous_total_sous_ib = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Devise_ib" ))
				    etat -> devise_de_calcul_ib = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_nom_ib" ))
				    etat -> afficher_nom_ib = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Tiers" ))
				    etat -> utilise_tiers = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Detail_tiers" ))
				    etat -> utilise_detail_tiers = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "No_tiers" )
				     &&
				     xmlNodeGetContent ( node_detail_etat ))
				{
				    gchar **pointeur_char;
				    gint i;

				    pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail_etat ),
								 "/",
								 0 );
				    i=0;

				    while ( pointeur_char[i] )
				    {
					etat -> no_tiers = g_slist_append ( etat -> no_tiers,
									    GINT_TO_POINTER ( my_atoi ( pointeur_char[i] )));
					i++;
				    }
				    g_strfreev ( pointeur_char );
				}

				if ( !strcmp ( node_detail_etat -> name,
					       "Total_tiers" ))
				    etat -> affiche_sous_total_tiers = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Devise_tiers" ))
				    etat -> devise_de_calcul_tiers = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Aff_nom_tiers" ))
				    etat -> afficher_nom_tiers = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Texte" ))
				    etat -> utilise_texte = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Texte_comp" ))
				{
				    xmlNodePtr node_comp_textes;

				    node_comp_textes = node_detail_etat -> children;

				    /*  on fait le tour des comparaisons */

				    while ( node_comp_textes )
				    {
					struct struct_comparaison_textes_etat *comp_textes;

					comp_textes = calloc ( 1,
							       sizeof ( struct struct_comparaison_textes_etat ));

					if ( node_comp_textes -> type != XML_TEXT_NODE )
					{
					    comp_textes -> lien_struct_precedente = my_atoi ( xmlGetProp ( node_comp_textes,
													   "Lien_struct" ));
					    comp_textes -> champ = my_atoi ( xmlGetProp ( node_comp_textes,
											  "Champ" ));
					    comp_textes -> operateur = my_atoi ( xmlGetProp ( node_comp_textes,
											      "Op" ));
					    comp_textes -> texte = xmlGetProp ( node_comp_textes,
										"Txt" );
					    comp_textes -> utilise_txt = my_atoi ( xmlGetProp ( node_comp_textes,
												"Util_txt" ));
					    comp_textes -> comparateur_1 = my_atoi ( xmlGetProp ( node_comp_textes,
												  "Comp_1" ));
					    comp_textes -> lien_1_2 = my_atoi ( xmlGetProp ( node_comp_textes,
											     "Lien_1_2" ));
					    comp_textes -> comparateur_2 = my_atoi ( xmlGetProp ( node_comp_textes,
												  "Comp_2" ));
					    comp_textes -> montant_1 = my_atoi ( xmlGetProp ( node_comp_textes,
											      "Mont_1" ));
					    comp_textes -> montant_2 = my_atoi ( xmlGetProp ( node_comp_textes,
											      "Mont_2" ));

					    /* on a fini de remplir le détail de la comparaison, on l'ajoute à la liste */

					    etat -> liste_struct_comparaison_textes = g_slist_append ( etat -> liste_struct_comparaison_textes,
												       comp_textes );
					}
					node_comp_textes = node_comp_textes -> next;
				    }
				}

				if ( !strcmp ( node_detail_etat -> name,
					       "Montant" ))
				    etat -> utilise_montant = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Montant_devise" ))
				    etat -> choix_devise_montant = my_atoi ( xmlNodeGetContent ( node_detail_etat ));


				if ( !strcmp ( node_detail_etat -> name,
					       "Montant_comp" ))
				{
				    xmlNodePtr node_comp_montants;

				    node_comp_montants = node_detail_etat -> children;

				    /*  on fait le tour des comparaisons */

				    while ( node_comp_montants )
				    {
					struct struct_comparaison_montants_etat *comp_montants;

					comp_montants = calloc ( 1,
								 sizeof ( struct struct_comparaison_montants_etat ));


					if ( node_comp_montants -> type != XML_TEXT_NODE )
					{
					    comp_montants -> lien_struct_precedente = my_atoi ( xmlGetProp ( node_comp_montants,
													     "Lien_struct" ));
					    comp_montants -> comparateur_1 = my_atoi ( xmlGetProp ( node_comp_montants,
												    "Comp_1" ));
					    comp_montants -> lien_1_2 = my_atoi ( xmlGetProp ( node_comp_montants,
											       "Lien_1_2" ));
					    comp_montants -> comparateur_2 = my_atoi ( xmlGetProp ( node_comp_montants,
												    "Comp_2" ));
					    comp_montants -> montant_1 = my_strtod ( xmlGetProp ( node_comp_montants,
												  "Mont_1" ),
										     NULL );
					    comp_montants -> montant_2 = my_strtod ( xmlGetProp ( node_comp_montants,
												  "Mont_2" ),
										     NULL );

					    /* on a fini de remplir le détail de la comparaison, on l'ajoute à la liste */

					    etat -> liste_struct_comparaison_montants = g_slist_append ( etat -> liste_struct_comparaison_montants,
													 comp_montants );
					}
					node_comp_montants = node_comp_montants -> next;
				    }
				}


				if ( !strcmp ( node_detail_etat -> name,
					       "Excl_nul" ))
				    etat -> exclure_montants_nuls = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Detail_mod_paie" ))
				    etat -> utilise_mode_paiement = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

				if ( !strcmp ( node_detail_etat -> name,
					       "Liste_mod_paie" ))
				{
				    xmlNodePtr node_mode_paiement;

				    node_mode_paiement = node_detail_etat -> children;

				    /*  on fait le tour des modes de paiement */

				    while ( node_mode_paiement )
				    {
					if ( node_mode_paiement -> type != XML_TEXT_NODE )
					    etat -> noms_modes_paiement = g_slist_append ( etat -> noms_modes_paiement,
											   xmlGetProp ( node_mode_paiement,
													"Nom" ));
					node_mode_paiement = node_mode_paiement -> next;
				    }
				}
			    }
			    node_detail_etat = node_detail_etat -> next;
			}

			/* on a fini de remplir l'état, on l'ajoute à la liste */
			liste_struct_etats = g_slist_append ( liste_struct_etats, etat );
		    }
		    node_detail = node_detail -> next;
		}
	    }
	}
	node_etats = node_etats -> next;
    }
    return TRUE;
}
/***********************************************************************************************************/




/***********************************************************************************************************/
/* Procedure qui sauvegarde le fichier */
/* retourne TRUE : no problem */
/* retourne FALSE : pas de nom => gtkfileselection a pris la main ou erreur */
/***********************************************************************************************************/

gboolean enregistre_fichier ( gchar *nouveau_fichier )
{
    xmlDocPtr doc;
    xmlNodePtr node;
    xmlNodePtr node_1;
    gint resultat;
    gchar *pointeur_char;
    GSList *pointeur_liste;
    GList *pointeur_list;
    gint i, j;
    GSList *pointeur_liste_2;
    struct stat buffer_stat;
    gint mettre_permission;


    if ( DEBUG )
	printf ( "enregistre_fichier : %s\n", nouveau_fichier );

    /*     on ne se préocupe plus des ouvertures et autre à ce niveau */
    /*     cette fonction enregistre le fichier sans avertissement(écrasement, fichier déjà ouvert...) */
    /* 	tout doit être fait avant */

    
    /* on regarde ici si le fichier existe */
    /*   s'il n'existe pas on mettre ses permissions à 600, sinon on les laisse comme ça */

    if ( stat ( nouveau_fichier,
		&buffer_stat ) == -1 )
	mettre_permission = 1;
    else
	mettre_permission = 0;


    etat.en_train_de_sauvegarder = 1;

    /* creation de l'arbre xml en memoire */

    doc = xmlNewDoc("1.0");

    /* la racine est grisbi */

    doc->children = xmlNewDocNode ( doc,
				    NULL,
				    "Grisbi",
				    NULL );

    /* on commence à ajouter les generalites */

    node = xmlNewChild ( doc->children,
			 NULL,
			 "Generalites",
			 NULL );
    xmlNewTextChild ( node,
		      NULL,
		      "Version_fichier",
		      VERSION_FICHIER );

    xmlNewTextChild ( node,
		      NULL,
		      "Version_grisbi",
		      VERSION );

    xmlNewTextChild ( node,
		      NULL,
		      "Backup",
		      nom_fichier_backup );
    xmlNewTextChild ( node,
		      NULL,
		      "Titre",
		      titre_fichier);
    xmlNewTextChild ( node,
		      NULL,
		      "Adresse_commune",
		      adresse_commune);
    xmlNewTextChild ( node,
		      NULL,
		      "Adresse_secondaire",
		      adresse_secondaire);
    xmlNewTextChild ( node,
		      NULL,
		      "Numero_devise_totaux_tiers",
		      itoa ( no_devise_totaux_tiers));
    xmlNewTextChild ( node,
		      NULL,
		      "Type_affichage_des_echeances",
		      itoa (affichage_echeances));
    xmlNewTextChild ( node,
		      NULL,
		      "Affichage_echeances_perso_nb_libre",
		      itoa (affichage_echeances_perso_nb_libre));
    xmlNewTextChild ( node,
		      NULL,
		      "Type_affichage_perso_echeances",
		      itoa (affichage_echeances_perso_j_m_a));
    xmlNewTextChild ( node,
		      NULL,
		      "Numero_derniere_operation",
		      itoa ( no_derniere_operation));

    xmlNewTextChild ( node,
		      NULL,
		      "Echelle_date_import",
		      itoa (valeur_echelle_recherche_date_import ));

    xmlNewTextChild ( node,
		      NULL,
		      "Utilise_logo",
		      itoa (etat.utilise_logo ));

    xmlNewTextChild ( node,
		      NULL,
		      "Chemin_logo",
		      chemin_logo );

    xmlNewChild ( node,
		  NULL,
		  "Caracteristiques_par_compte",
		  itoa(etat.retient_affichage_par_compte));

    /* creation de l'ordre de l'affichage des opés */

    pointeur_char = NULL;

    for ( i=0 ; i<4 ; i++ )
	for ( j=0 ; j< 7 ; j++ )
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "-",
					      itoa ( tab_affichage_ope[i][j] ),
					      NULL );
	    else
		pointeur_char = itoa ( tab_affichage_ope[i][j] );


    xmlNewTextChild ( node,
		      NULL,
		      "Affichage_opes",
		      pointeur_char );

    /* creation des rapport largeurs colonnes */

    pointeur_char = NULL;

    for ( i=0 ; i<7 ; i++ )
	if ( pointeur_char )
	    pointeur_char = g_strconcat ( pointeur_char,
					  "-",
					  itoa ( rapport_largeur_colonnes[i] ),
					  NULL );
	else
	    pointeur_char = itoa ( rapport_largeur_colonnes[i] );


    xmlNewTextChild ( node,
		      NULL,
		      "Rapport_largeur_col",
		      pointeur_char );

    /* creation des lignes affichées par caractéristiques */

    xmlNewTextChild ( node,
		      NULL,
		      "Ligne_aff_une_ligne",
		      itoa ( ligne_affichage_une_ligne ));


    pointeur_char = g_strconcat ( itoa ( GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> data )),
				  "-",
				  itoa ( GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> next -> data )),
				  NULL );
    xmlNewTextChild ( node,
		      NULL,
		      "Lignes_aff_deux_lignes",
		      pointeur_char );

    pointeur_char = g_strconcat ( itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> data )),
				  "-",
				  itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> data )),
				  "-",
				  itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> next -> data )),
				  NULL );
    xmlNewTextChild ( node,
		      NULL,
		      "Lignes_aff_trois_lignes",
		      pointeur_char );


    /*     si on utilise un formulaire différent pour chaque compte */

    xmlNewChild ( node,
		  NULL,
		  "Formulaire_distinct_par_compte",
		  itoa(etat.formulaire_distinct_par_compte));

    /* creation des rapport largeurs colonnes de l'échéancier */

    pointeur_char = NULL;

    for ( i=0 ; i<NB_COLS_SCHEDULER ; i++ )
	if ( pointeur_char )
	    pointeur_char = g_strconcat ( pointeur_char,
					  "-",
					  itoa ( scheduler_col_width[i] ),
					  NULL );
	else
	    pointeur_char = itoa ( scheduler_col_width[i] );


    xmlNewTextChild ( node,
		      NULL,
		      "Rapport_largeur_col_echeancier",
		      pointeur_char );

    /*   on commence la sauvegarde des comptes : 2 parties, les generalites */
    /* puis les comptes 1 par 1 */

    node = xmlNewChild ( doc->children,
			 NULL,
			 "Comptes",
			 NULL );

    /* c'est node_1 qui va contenir les generalites puis les comptes */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Generalites",
			   NULL );

    /* creation de l'ordre des comptes */

    pointeur_liste = ordre_comptes;
    pointeur_char = NULL;

    do
    {
	if ( pointeur_char )
	    pointeur_char = g_strconcat ( pointeur_char,
					  "-",
					  itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					  NULL );
	else
	    pointeur_char = itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	pointeur_liste = pointeur_liste -> next;
    }
    while ( pointeur_liste );

    xmlNewTextChild ( node_1,
		      NULL,
		      "Ordre_des_comptes",
		      pointeur_char );

    xmlNewTextChild ( node_1,
		      NULL,
		      "Compte_courant",
		      itoa ( compte_courant ) );


    /* mise en place des comptes 1 par 1 */

    for ( i=0 ; i < nb_comptes ; i++)
    {
	xmlNodePtr node_compte;
	gint k;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	node_1 = xmlNewChild ( node,
			       NULL,
			       "Compte",
			       NULL );

	/* on met d'abord le detail du compte, les types puis les operations */

	/* mise en forme de la date du dernier releve */

	if ( DATE_DERNIER_RELEVE )
	    pointeur_char = g_strconcat ( itoa ( g_date_day ( DATE_DERNIER_RELEVE ) ),
					  "/",
					  itoa ( g_date_month ( DATE_DERNIER_RELEVE ) ),
					  "/",
					  itoa ( g_date_year ( DATE_DERNIER_RELEVE ) ),
					  NULL );
	else
	    pointeur_char = NULL;


	node_compte = xmlNewChild ( node_1,
				    NULL,
				    "Details",
				    NULL );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Nom",
			  NOM_DU_COMPTE );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Id_compte",
			  ID_COMPTE );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "No_de_compte",
			  itoa ( NO_COMPTE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Titulaire",
			  TITULAIRE );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Type_de_compte",
			  itoa ( TYPE_DE_COMPTE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Devise",
			  itoa ( DEVISE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Banque",
			  itoa ( BANQUE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Guichet",
			  NO_GUICHET );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "No_compte_banque",
			  NO_COMPTE_BANQUE );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Cle_du_compte",
			  CLE_COMPTE );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Solde_initial",
			  g_strdup_printf ( "%4.7f",
					    SOLDE_INIT ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Solde_mini_voulu",
			  g_strdup_printf ( "%4.7f",
					    SOLDE_MINI_VOULU ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Solde_mini_autorise",
			  g_strdup_printf ( "%4.7f",
					    SOLDE_MINI ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Date_dernier_releve",
			  pointeur_char );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Solde_dernier_releve",
			  g_strdup_printf ( "%4.7f",
					    SOLDE_DERNIER_RELEVE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Dernier_no_de_rapprochement",
			  itoa ( DERNIER_NO_RAPPROCHEMENT ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Compte_cloture",
			  itoa ( COMPTE_CLOTURE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Affichage_r",
			  itoa ( AFFICHAGE_R ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Nb_lignes_ope",
			  itoa ( NB_LIGNES_OPE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Commentaires",
			  COMMENTAIRE );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Adresse_du_titulaire",
			  ADRESSE_TITULAIRE );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Nombre_de_types",
			  itoa ( g_slist_length ( TYPES_OPES ) ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Type_defaut_debit",
			  itoa ( TYPE_DEFAUT_DEBIT ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Type_defaut_credit",
			  itoa ( TYPE_DEFAUT_CREDIT ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Tri_par_type",
			  itoa ( TRI ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Neutres_inclus",
			  itoa ( NEUTRES_INCLUS ));

	/* mise en place de l'ordre du tri */


	pointeur_liste = LISTE_TRI;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Ordre_du_tri",
			  pointeur_char );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Classement_croissant",
			  itoa ( CLASSEMENT_CROISSANT ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "No_classement",
			  itoa ( NO_CLASSEMENT ));

	/* 	on sauvegarde l'agencement du formulaire */

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Nb_colonnes_formulaire",
			  itoa ( ORGANISATION_FORMULAIRE -> nb_colonnes ));
	xmlNewTextChild ( node_compte,
			  NULL,
			  "Nb_lignes_formulaire",
			  itoa ( ORGANISATION_FORMULAIRE -> nb_lignes ));

	pointeur_char = NULL;

	for ( k=0 ; k<4 ; k++ )
	    for ( j=0 ; j< 6 ; j++ )
		if ( pointeur_char )
		    pointeur_char = g_strconcat ( pointeur_char,
						  "-",
						  itoa ( ORGANISATION_FORMULAIRE -> tab_remplissage_formulaire [k][j] ),
						  NULL );
		else
		    pointeur_char = itoa ( ORGANISATION_FORMULAIRE -> tab_remplissage_formulaire [k][j] );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Organisation_formulaire",
			  pointeur_char );

	pointeur_char = NULL;

	for ( k=0 ; k<6 ; k++ )
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "-",
					      itoa ( ORGANISATION_FORMULAIRE -> taille_colonne_pourcent [k] ),
					      NULL );
	    else
		pointeur_char = itoa ( ORGANISATION_FORMULAIRE -> taille_colonne_pourcent [k] );


	xmlNewTextChild ( node_compte,
			  NULL,
			  "Largeur_col_formulaire",
			  pointeur_char );



	
	/* mise en place des types */

	node_compte = xmlNewChild ( node_1,
				    NULL,
				    "Detail_de_Types",
				    NULL );

	pointeur_liste = TYPES_OPES;

	while ( pointeur_liste )
	{
	    struct struct_type_ope *type;
	    xmlNodePtr node_type;

	    node_type = xmlNewChild ( node_compte,
				      NULL,
				      "Type",
				      NULL );

	    type = pointeur_liste -> data;


	    xmlSetProp ( node_type,
			 "No",
			 itoa ( type -> no_type ));

	    xmlSetProp ( node_type,
			 "Nom",
			 type -> nom_type );

	    xmlSetProp ( node_type,
			 "Signe",
			 itoa ( type -> signe_type ));

	    xmlSetProp ( node_type,
			 "Affiche_entree",
			 itoa ( type -> affiche_entree ));

	    xmlSetProp ( node_type,
			 "Numerotation_auto",
			 itoa ( type -> numerotation_auto ));

	    xmlSetProp ( node_type,
			 "No_en_cours",
			 itoa ( type -> no_en_cours ));

	    pointeur_liste = pointeur_liste -> next;
	}


	/* mise en place des operations */

	node_compte = xmlNewChild ( node_1,
				    NULL,
				    "Detail_des_operations",
				    NULL );

	pointeur_liste = LISTE_OPERATIONS;

	while ( pointeur_liste )
	{
	    struct structure_operation *operation;
	    xmlNodePtr node_ope;

	    operation = pointeur_liste -> data;

	    node_ope = xmlNewChild ( node_compte,
				     NULL,
				     "Operation",
				     NULL );

	    xmlSetProp ( node_ope,
			 "No",
			 itoa ( operation -> no_operation ));

	    xmlSetProp ( node_ope,
			 "Id",
			 operation -> id_operation );

	    xmlSetProp ( node_ope,
			 "D",
			 g_strdup_printf ( "%d/%d/%d",
					   operation -> jour,
					   operation -> mois,
					   operation -> annee ));

	    /* GDC : Ecriture de la date bancaire */
	    xmlSetProp ( node_ope,
			 "Db",
			 g_strdup_printf ( "%d/%d/%d",
					   operation -> jour_bancaire,
					   operation -> mois_bancaire,
					   operation -> annee_bancaire ));
	    /* GDCFin */

	    xmlSetProp ( node_ope,
			 "M",
			 g_strdup_printf ( "%4.7f",
					   operation -> montant ));

	    xmlSetProp ( node_ope,
			 "De",
			 itoa ( operation -> devise ));

	    xmlSetProp ( node_ope,
			 "Rdc",
			 itoa ( operation -> une_devise_compte_egale_x_devise_ope ));

	    xmlSetProp ( node_ope,
			 "Tc",
			 g_strdup_printf ( "%4.7f",
					   operation -> taux_change ));

	    xmlSetProp ( node_ope,
			 "Fc",
			 g_strdup_printf ( "%4.7f",
					   operation -> frais_change ));

	    xmlSetProp ( node_ope,
			 "T",
			 itoa ( operation -> tiers ));

	    xmlSetProp ( node_ope,
			 "C",
			 itoa ( operation -> categorie ));

	    xmlSetProp ( node_ope,
			 "Sc",
			 itoa ( operation -> sous_categorie ));

	    xmlSetProp ( node_ope,
			 "Ov",
			 itoa ( operation -> operation_ventilee ));

	    xmlSetProp ( node_ope,
			 "N",
			 operation -> notes );

	    xmlSetProp ( node_ope,
			 "Ty",
			 itoa ( operation -> type_ope ));

	    xmlSetProp ( node_ope,
			 "Ct",
			 operation -> contenu_type );

	    xmlSetProp ( node_ope,
			 "P",
			 itoa ( operation -> pointe ));

	    xmlSetProp ( node_ope,
			 "A",
			 itoa ( operation -> auto_man ));

	    xmlSetProp ( node_ope,
			 "R",
			 itoa ( operation -> no_rapprochement ));

	    xmlSetProp ( node_ope,
			 "E",
			 itoa ( operation -> no_exercice ));

	    xmlSetProp ( node_ope,
			 "I",
			 itoa ( operation -> imputation ));

	    xmlSetProp ( node_ope,
			 "Si",
			 itoa ( operation -> sous_imputation ));

	    xmlSetProp ( node_ope,
			 "Pc",
			 operation -> no_piece_comptable );

	    xmlSetProp ( node_ope,
			 "Ibg",
			 operation -> info_banque_guichet );

	    xmlSetProp ( node_ope,
			 "Ro",
			 itoa ( operation -> relation_no_operation ));

	    xmlSetProp ( node_ope,
			 "Rc",
			 itoa ( operation -> relation_no_compte ));

	    xmlSetProp ( node_ope,
			 "Va",
			 itoa ( operation -> no_operation_ventilee_associee ));

	    pointeur_liste = pointeur_liste -> next;
	}
    }


    /* on sauvegarde maintenant les echeances : generalites puis liste des echeances */


    node = xmlNewChild ( doc->children,
			 NULL,
			 "Echeances",
			 NULL );

    /* c'est node_1 qui va contenir les generalites */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Generalites",
			   NULL );

    xmlNewTextChild ( node_1,
		      NULL,
		      "Nb_echeances",
		      itoa ( nb_echeances ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_derniere_echeance",
		      itoa ( no_derniere_echeance ));

    /* on met les echeances */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_echeances",
			   NULL );

    pointeur_liste = liste_struct_echeances;

    while ( pointeur_liste )
    {
	struct operation_echeance *echeance;
	xmlNodePtr node_echeance;

	echeance = pointeur_liste -> data;


	node_echeance = xmlNewChild ( node_1,
				      NULL,
				      "Echeance",
				      NULL );

	xmlSetProp ( node_echeance,
		     "No",
		     itoa ( echeance -> no_operation ));

	xmlSetProp ( node_echeance,
		     "Date",
		     g_strdup_printf ( "%d/%d/%d",
				       echeance -> jour,
				       echeance -> mois,
				       echeance -> annee ));

	xmlSetProp ( node_echeance,
		     "Compte",
		     itoa ( echeance -> compte ));

	xmlSetProp ( node_echeance,
		     "Montant",
		     g_strdup_printf ( "%4.7f",
				       echeance -> montant ));

	xmlSetProp ( node_echeance,
		     "Devise",
		     itoa ( echeance -> devise ));

	xmlSetProp ( node_echeance,
		     "Tiers",
		     itoa ( echeance -> tiers ));

	xmlSetProp ( node_echeance,
		     "Categorie",
		     itoa ( echeance -> categorie ));

	xmlSetProp ( node_echeance,
		     "Sous-categorie",
		     itoa ( echeance -> sous_categorie ));

	xmlSetProp ( node_echeance,
		     "Virement_compte",
		     itoa ( echeance -> compte_virement ));

	xmlSetProp ( node_echeance,
		     "Type",
		     itoa ( echeance -> type_ope ));

	xmlSetProp ( node_echeance,
		     "Type_contre_ope",
		     itoa ( echeance -> type_contre_ope ));

	xmlSetProp ( node_echeance,
		     "Contenu_du_type",
		     echeance -> contenu_type );

	xmlSetProp ( node_echeance,
		     "Exercice",
		     itoa ( echeance -> no_exercice ));

	xmlSetProp ( node_echeance,
		     "Imputation",
		     itoa ( echeance -> imputation ));

	xmlSetProp ( node_echeance,
		     "Sous-imputation",
		     itoa ( echeance -> sous_imputation ));

	xmlSetProp ( node_echeance,
		     "Notes",
		     echeance -> notes );

	xmlSetProp ( node_echeance,
		     "Automatique",
		     itoa ( echeance -> auto_man ));

	xmlSetProp ( node_echeance,
		     "Periodicite",
		     itoa ( echeance -> periodicite ));

	xmlSetProp ( node_echeance,
		     "Intervalle_periodicite",
		     itoa ( echeance -> intervalle_periodicite_personnalisee ));

	xmlSetProp ( node_echeance,
		     "Periodicite_personnalisee",
		     itoa ( echeance -> periodicite_personnalisee ));

	if ( echeance -> jour_limite )
	    xmlSetProp ( node_echeance,
			 "Date_limite",
			 g_strdup_printf ( "%d/%d/%d",
					   echeance -> jour_limite,
					   echeance -> mois_limite,
					   echeance -> annee_limite ));
	else
	    xmlSetProp ( node_echeance,
			 "Date_limite",
			 NULL );

	xmlSetProp ( node_echeance,
		     "Ech_ventilee",
		     itoa ( echeance -> operation_ventilee ));

	xmlSetProp ( node_echeance,
		     "No_ech_associee",
		     itoa ( echeance -> no_operation_ventilee_associee ));


	pointeur_liste = pointeur_liste -> next;
    }


    /* mise en place des tiers : generalites puis liste */


    node = xmlNewChild ( doc->children,
			 NULL,
			 "Tiers",
			 NULL );

    /* c'est node_1 qui va contenir les generalites */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Generalites",
			   NULL );

    xmlNewTextChild ( node_1,
		      NULL,
		      "Nb_tiers",
		      itoa ( nb_enregistrements_tiers ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_dernier_tiers",
		      itoa ( no_dernier_tiers ));

    /* on met les tiers */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_tiers",
			   NULL );

    pointeur_liste = liste_struct_tiers;

    while ( pointeur_liste )
    {
	struct struct_tiers *tiers;
	xmlNodePtr node_tiers;

	tiers = pointeur_liste -> data;


	node_tiers = xmlNewChild ( node_1,
				   NULL,
				   "Tiers",
				   NULL );

	xmlSetProp ( node_tiers,
		     "No",
		     itoa ( tiers -> no_tiers ));

	xmlSetProp ( node_tiers,
		     "Nom",
		     tiers -> nom_tiers );

	xmlSetProp ( node_tiers,
		     "Informations",
		     tiers -> texte );

	xmlSetProp ( node_tiers,
		     "Liaison",
		     itoa ( tiers -> liaison ));

	pointeur_liste = pointeur_liste -> next;
    }


    /* mise en place des categories : generalites puis liste */


    node = xmlNewChild ( doc->children,
			 NULL,
			 "Categories",
			 NULL );

    /* c'est node_1 qui va contenir les generalites */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Generalites",
			   NULL );

    xmlNewTextChild ( node_1,
		      NULL,
		      "Nb_categories",
		      itoa ( nb_enregistrements_categories ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_derniere_categorie",
		      itoa ( no_derniere_categorie ));

    /* on met les categories */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_categories",
			   NULL );

    pointeur_liste = liste_struct_categories;

    while ( pointeur_liste )
    {
	struct struct_categ *categ;
	xmlNodePtr node_categ;
	GSList *pointeur_sous_categ;

	categ = pointeur_liste -> data;


	node_categ = xmlNewChild ( node_1,
				   NULL,
				   "Categorie",
				   NULL );

	xmlSetProp ( node_categ,
		     "No",
		     itoa ( categ -> no_categ ));

	xmlSetProp ( node_categ,
		     "Nom",
		     categ -> nom_categ );

	xmlSetProp ( node_categ,
		     "Type",
		     itoa ( categ -> type_categ ));

	xmlSetProp ( node_categ,
		     "No_derniere_sous_cagegorie",
		     itoa ( categ -> no_derniere_sous_categ ));


	/* on ajoute les sous categories */

	pointeur_sous_categ = categ -> liste_sous_categ;

	while ( pointeur_sous_categ )
	{
	    struct struct_sous_categ *sous_categ;
	    xmlNodePtr node_sous_categ;

	    sous_categ = pointeur_sous_categ -> data;

	    node_sous_categ = xmlNewChild ( node_categ,
					    NULL,
					    "Sous-categorie",
					    NULL );

	    xmlSetProp ( node_sous_categ,
			 "No",
			 itoa ( sous_categ -> no_sous_categ ));

	    xmlSetProp ( node_sous_categ,
			 "Nom",
			 sous_categ -> nom_sous_categ );

	    pointeur_sous_categ = pointeur_sous_categ -> next;
	}
	pointeur_liste = pointeur_liste -> next;
    }


    /* mise en place des imputations budgetaires : generalites puis liste */


    node = xmlNewChild ( doc->children,
			 NULL,
			 "Imputations",
			 NULL );

    /* c'est node_1 qui va contenir les generalites */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Generalites",
			   NULL );

    xmlNewTextChild ( node_1,
		      NULL,
		      "Nb_imputations",
		      itoa ( nb_enregistrements_imputations ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_derniere_imputation",
		      itoa ( no_derniere_imputation ));

    /* on met les categories */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_imputations",
			   NULL );

    pointeur_liste = liste_struct_imputation;

    while ( pointeur_liste )
    {
	struct struct_imputation *imputation;
	xmlNodePtr node_imputation;
	GSList *pointeur_sous_imputation;

	imputation = pointeur_liste -> data;


	node_imputation = xmlNewChild ( node_1,
					NULL,
					"Imputation",
					NULL );

	xmlSetProp ( node_imputation,
		     "No",
		     itoa ( imputation -> no_imputation ));

	xmlSetProp ( node_imputation,
		     "Nom",
		     imputation -> nom_imputation );

	xmlSetProp ( node_imputation,
		     "Type",
		     itoa ( imputation -> type_imputation ));

	xmlSetProp ( node_imputation,
		     "No_derniere_sous_imputation",
		     itoa ( imputation -> no_derniere_sous_imputation ));

	/* on ajoute les sous ib */

	pointeur_sous_imputation = imputation -> liste_sous_imputation;

	while ( pointeur_sous_imputation )
	{
	    struct struct_sous_imputation *sous_imputation;
	    xmlNodePtr node_sous_imputation;

	    sous_imputation = pointeur_sous_imputation -> data;

	    node_sous_imputation = xmlNewChild ( node_imputation,
						 NULL,
						 "Sous-imputation",
						 NULL );

	    xmlSetProp ( node_sous_imputation,
			 "No",
			 itoa ( sous_imputation -> no_sous_imputation ));

	    xmlSetProp ( node_sous_imputation,
			 "Nom",
			 sous_imputation -> nom_sous_imputation );

	    pointeur_sous_imputation = pointeur_sous_imputation -> next;
	}
	pointeur_liste = pointeur_liste -> next;
    }


    /* mise en place des devises : generalites puis liste */


    node = xmlNewChild ( doc->children,
			 NULL,
			 "Devises",
			 NULL );

    /* c'est node_1 qui va contenir les generalites */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Generalites",
			   NULL );

    xmlNewTextChild ( node_1,
		      NULL,
		      "Nb_devises",
		      itoa ( nb_devises ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_derniere_devise",
		      itoa ( no_derniere_devise ));

    /* on met les devises */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_devises",
			   NULL );

    pointeur_liste = liste_struct_devises;

    while ( pointeur_liste )
    {
	struct struct_devise *devise;
	xmlNodePtr node_devise;

	devise = pointeur_liste -> data;


	node_devise = xmlNewChild ( node_1,
				    NULL,
				    "Devise",
				    NULL );

	xmlSetProp ( node_devise,
		     "No",
		     itoa ( devise -> no_devise ));

	xmlSetProp ( node_devise,
		     "Nom",
		     devise -> nom_devise );

	xmlSetProp ( node_devise,
		     "Code",
		     devise -> code_devise );

	xmlSetProp ( node_devise,
		     "IsoCode",
		     devise -> code_iso4217_devise );

	xmlSetProp ( node_devise,
		     "Passage_euro",
		     itoa ( devise -> passage_euro ));

	if ( devise -> date_dernier_change )
	    xmlSetProp ( node_devise,
			 "Date_dernier_change",
			 g_strdup_printf (  "%d/%d/%d",
					    g_date_day ( devise -> date_dernier_change ),
					    g_date_month ( devise -> date_dernier_change ),
					    g_date_year ( devise -> date_dernier_change )));
	else
	    xmlSetProp ( node_devise,
			 "Date_dernier_change",
			 NULL );

	xmlSetProp ( node_devise,
		     "Rapport_entre_devises",
		     itoa ( devise -> une_devise_1_egale_x_devise_2 ));

	xmlSetProp ( node_devise,
		     "Devise_en_rapport",
		     itoa ( devise -> no_devise_en_rapport ));

	xmlSetProp ( node_devise,
		     "Change",
		     g_strdup_printf ( "%f",
				       devise -> change ));

	pointeur_liste = pointeur_liste -> next;
    }


    /* mise en place des banques : generalites puis liste */


    node = xmlNewChild ( doc->children,
			 NULL,
			 "Banques",
			 NULL );

    /* c'est node_1 qui va contenir les generalites */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Generalites",
			   NULL );

    xmlNewTextChild ( node_1,
		      NULL,
		      "Nb_banques",
		      itoa ( nb_banques ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_derniere_banque",
		      itoa ( no_derniere_banque ));

    /* on met les banques */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_banques",
			   NULL );

    pointeur_liste = liste_struct_banques;

    while ( pointeur_liste )
    {
	struct struct_banque *banque;
	xmlNodePtr node_banque;

	banque = pointeur_liste -> data;


	node_banque = xmlNewChild ( node_1,
				    NULL,
				    "Banque",
				    NULL );

	xmlSetProp ( node_banque,
		     "No",
		     itoa ( banque -> no_banque ));

	xmlSetProp ( node_banque,
		     "Nom",
		     banque -> nom_banque );

	xmlSetProp ( node_banque,
		     "Code",
		     banque -> code_banque );

	xmlSetProp ( node_banque,
		     "Adresse",
		     banque -> adr_banque );

	xmlSetProp ( node_banque,
		     "Tel",
		     banque -> tel_banque );

	xmlSetProp ( node_banque,
		     "Mail",
		     banque -> email_banque );

	xmlSetProp ( node_banque,
		     "Web",
		     banque -> web_banque );

	xmlSetProp ( node_banque,
		     "Nom_correspondant",
		     banque -> nom_correspondant );

	xmlSetProp ( node_banque,
		     "Fax_correspondant",
		     banque -> fax_correspondant );

	xmlSetProp ( node_banque,
		     "Tel_correspondant",
		     banque -> tel_correspondant );

	xmlSetProp ( node_banque,
		     "Mail_correspondant",
		     banque -> email_correspondant );

	xmlSetProp ( node_banque,
		     "Remarques",
		     banque -> remarque_banque );

	pointeur_liste = pointeur_liste -> next;
    }

    /* mise en place des exercices : generalites puis liste */


    node = xmlNewChild ( doc->children,
			 NULL,
			 "Exercices",
			 NULL );

    /* c'est node_1 qui va contenir les generalites */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Generalites",
			   NULL );

    xmlNewTextChild ( node_1,
		      NULL,
		      "Nb_exercices",
		      itoa ( nb_exercices ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_dernier_exercice",
		      itoa ( no_derniere_exercice ));

    /* on met les exercices */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_exercices",
			   NULL );

    pointeur_liste = liste_struct_exercices;

    while ( pointeur_liste )
    {
	struct struct_exercice *exercice;
	xmlNodePtr node_exercice;

	exercice = pointeur_liste -> data;


	node_exercice = xmlNewChild ( node_1,
				      NULL,
				      "Exercice",
				      NULL );

	xmlSetProp ( node_exercice,
		     "No",
		     itoa ( exercice -> no_exercice ));

	xmlSetProp ( node_exercice,
		     "Nom",
		     exercice -> nom_exercice );

	if ( exercice->date_debut )
	    xmlSetProp ( node_exercice,
			 "Date_debut",
			 g_strdup_printf ( "%d/%d/%d",
					   g_date_day ( exercice->date_debut ),
					   g_date_month ( exercice->date_debut ),
					   g_date_year ( exercice->date_debut )));
	else
	    xmlSetProp ( node_exercice,
			 "Date_debut",
			 NULL );

	if ( exercice->date_fin )
	    xmlSetProp ( node_exercice,
			 "Date_fin",
			 g_strdup_printf ( "%d/%d/%d",
					   g_date_day ( exercice->date_fin ),
					   g_date_month ( exercice->date_fin ),
					   g_date_year ( exercice->date_fin )));
	else
	    xmlSetProp ( node_exercice,
			 "Date_fin",
			 NULL );

	xmlSetProp ( node_exercice,
		     "Affiche",
		     itoa ( exercice -> affiche_dans_formulaire ));

	pointeur_liste = pointeur_liste -> next;
    }


    /* mise en place des no de rapprochement : generalites puis liste */


    node = xmlNewChild ( doc->children,
			 NULL,
			 "Rapprochements",
			 NULL );

    /* c'est node_1 qui va contenir les generalites */

    /* on n'a pas de generalites, peut être un jour ... */

    /*   node_1 = xmlNewChild ( node, */
    /* 			 NULL, */
    /* 			 "Generalites", */
    /* 			 NULL ); */

    /*   xmlNewTextChild ( node_1, */
    /* 		    NULL, */
    /* 		    "Nb_rapprochements", */
    /* 		    itoa ( g_slist_length ( liste_struct_rapprochements ))); */

    /* on met les rapprochements */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_rapprochements",
			   NULL );

    pointeur_liste = liste_struct_rapprochements;

    while ( pointeur_liste )
    {
	struct struct_no_rapprochement *rapprochement;
	xmlNodePtr node_rapprochement;

	rapprochement = pointeur_liste -> data;


	node_rapprochement = xmlNewChild ( node_1,
					   NULL,
					   "Rapprochement",
					   NULL );

	xmlSetProp ( node_rapprochement,
		     "No",
		     itoa ( rapprochement -> no_rapprochement ));

	xmlSetProp ( node_rapprochement,
		     "Nom",
		     rapprochement -> nom_rapprochement );

	pointeur_liste = pointeur_liste -> next;
    }


    /* sauvegarde des etats */

    node = xmlNewChild ( doc->children,
			 NULL,
			 "Etats",
			 NULL );

    /* c'est node_1 qui va contenir les generalites */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Generalites",
			   NULL );

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_dernier_etat",
		      itoa ( no_dernier_etat ));

    /* on met les etats */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_etats",
			   NULL );

    pointeur_liste_2 = liste_struct_etats;

    while ( pointeur_liste_2 )
    {
	struct struct_etat *etat;
	xmlNodePtr node_etat;
	xmlNodePtr node_2;

	etat = pointeur_liste_2 -> data;


	node_etat = xmlNewChild ( node_1,
				  NULL,
				  "Etat",
				  NULL );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No",
			  itoa ( etat -> no_etat ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Nom",
			  etat -> nom_etat );

	pointeur_list = etat -> type_classement;
	pointeur_char = NULL;

	while ( pointeur_list )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      itoa ( GPOINTER_TO_INT ( pointeur_list -> data )),
					      NULL );
	    else
		pointeur_char = itoa ( GPOINTER_TO_INT ( pointeur_list -> data ));

	    pointeur_list = pointeur_list -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Type_classement",
			  pointeur_char );


	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_r",
			  itoa ( etat -> afficher_r ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ope",
			  itoa ( etat -> afficher_opes ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_nb_ope",
			  itoa ( etat -> afficher_nb_opes ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_no_ope",
			  itoa ( etat -> afficher_no_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_date_ope",
			  itoa ( etat -> afficher_date_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_tiers_ope",
			  itoa ( etat -> afficher_tiers_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_categ_ope",
			  itoa ( etat -> afficher_categ_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ss_categ_ope",
			  itoa ( etat -> afficher_sous_categ_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_type_ope",
			  itoa ( etat -> afficher_type_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ib_ope",
			  itoa ( etat -> afficher_ib_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ss_ib_ope",
			  itoa ( etat -> afficher_sous_ib_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_cheque_ope",
			  itoa ( etat -> afficher_cheque_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_notes_ope",
			  itoa ( etat -> afficher_notes_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_pc_ope",
			  itoa ( etat -> afficher_pc_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_rappr_ope",
			  itoa ( etat -> afficher_rappr_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_infobd_ope",
			  itoa ( etat -> afficher_infobd_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_exo_ope",
			  itoa ( etat -> afficher_exo_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Class_ope",
			  itoa ( etat -> type_classement_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_titres_col",
			  itoa ( etat -> afficher_titre_colonnes ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_titres_chgt",
			  itoa ( etat -> type_affichage_titres ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Pas_detail_ventil",
			  itoa ( etat -> pas_detailler_ventilation ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Sep_rev_dep",
			  itoa ( etat -> separer_revenus_depenses ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Devise_gen",
			  itoa ( etat -> devise_de_calcul_general ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Incl_tiers",
			  itoa ( etat -> inclure_dans_tiers ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Ope_click",
			  itoa ( etat -> ope_clickables ));


	xmlNewTextChild ( node_etat,
			  NULL,
			  "Exo_date",
			  itoa ( etat -> exo_date ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_exo",
			  itoa ( etat -> utilise_detail_exo ));

	pointeur_liste = etat -> no_exercices;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_exo",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Plage_date",
			  itoa ( etat -> no_plage_date ));


	if ( etat->date_perso_debut )
	    xmlNewTextChild ( node_etat,
			      NULL,
			      "Date_debut",
			      g_strdup_printf ( "%d/%d/%d",
						g_date_day ( etat->date_perso_debut ),
						g_date_month ( etat->date_perso_debut ),
						g_date_year ( etat->date_perso_debut )));
	else
	    xmlNewTextChild ( node_etat,
			      NULL,
			      "Date_debut",
			      NULL );

	if ( etat->date_perso_fin )
	    xmlNewTextChild ( node_etat,
			      NULL,
			      "Date_fin",
			      g_strdup_printf ( "%d/%d/%d",
						g_date_day ( etat->date_perso_fin ),
						g_date_month ( etat->date_perso_fin ),
						g_date_year ( etat->date_perso_fin )));
	else
	    xmlNewTextChild ( node_etat,
			      NULL,
			      "Date_fin",
			      NULL );


	xmlNewTextChild ( node_etat,
			  NULL,
			  "Utilise_plages",
			  itoa ( etat -> separation_par_plage ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Sep_plages",
			  itoa ( etat -> type_separation_plage ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Sep_exo",
			  itoa ( etat -> separation_par_exo ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Deb_sem_plages",
			  itoa ( etat -> jour_debut_semaine ));


	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_comptes",
			  itoa ( etat -> utilise_detail_comptes ));

	pointeur_liste = etat -> no_comptes;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_comptes",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Grp_ope_compte",
			  itoa ( etat -> regroupe_ope_par_compte ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_compte",
			  itoa ( etat -> affiche_sous_total_compte ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_nom_compte",
			  itoa ( etat -> afficher_nom_compte ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Type_vir",
			  itoa ( etat -> type_virement ));

	pointeur_liste = etat -> no_comptes_virements;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_comptes_virements",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Exclure_non_vir",
			  itoa ( etat -> exclure_ope_non_virement ));



	xmlNewTextChild ( node_etat,
			  NULL,
			  "Categ",
			  itoa ( etat -> utilise_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_categ",
			  itoa ( etat -> utilise_detail_categ ));

	pointeur_liste = etat -> no_categ;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_categ",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Exclut_categ",
			  itoa ( etat -> exclure_ope_sans_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_categ",
			  itoa ( etat -> affiche_sous_total_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ss_categ",
			  itoa ( etat -> afficher_sous_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_pas_ss_categ",
			  itoa ( etat -> afficher_pas_de_sous_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_ss_categ",
			  itoa ( etat -> affiche_sous_total_sous_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Devise_categ",
			  itoa ( etat -> devise_de_calcul_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_nom_categ",
			  itoa ( etat -> afficher_nom_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "IB",
			  itoa ( etat -> utilise_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_ib",
			  itoa ( etat -> utilise_detail_ib ));

	pointeur_liste = etat -> no_ib;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_ib",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Exclut_ib",
			  itoa ( etat -> exclure_ope_sans_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_ib",
			  itoa ( etat -> affiche_sous_total_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ss_ib",
			  itoa ( etat -> afficher_sous_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_pas_ss_ib",
			  itoa ( etat -> afficher_pas_de_sous_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_ss_ib",
			  itoa ( etat -> affiche_sous_total_sous_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Devise_ib",
			  itoa ( etat -> devise_de_calcul_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_nom_ib",
			  itoa ( etat -> afficher_nom_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Tiers",
			  itoa ( etat -> utilise_tiers ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_tiers",
			  itoa ( etat -> utilise_detail_tiers ));

	pointeur_liste = etat -> no_tiers;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_tiers",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_tiers",
			  itoa ( etat -> affiche_sous_total_tiers ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Devise_tiers",
			  itoa ( etat -> devise_de_calcul_tiers ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_nom_tiers",
			  itoa ( etat -> afficher_nom_tiers ));


	xmlNewTextChild ( node_etat,
			  NULL,
			  "Texte",
			  itoa ( etat -> utilise_texte ));

	node_2 = xmlNewChild ( node_etat,
			       NULL,
			       "Texte_comp",
			       NULL );

	pointeur_liste = etat -> liste_struct_comparaison_textes;

	while ( pointeur_liste )
	{
	    struct struct_comparaison_textes_etat *textes_comp;
	    xmlNodePtr node_3;

	    textes_comp = pointeur_liste -> data;

	    node_3 = xmlNewChild ( node_2,
				   NULL,
				   "Comp",
				   NULL );
	    xmlSetProp ( node_3,
			 "Lien_struct",
			 itoa ( textes_comp -> lien_struct_precedente ));
	    xmlSetProp ( node_3,
			 "Champ",
			 itoa ( textes_comp -> champ ));
	    xmlSetProp ( node_3,
			 "Op",
			 itoa ( textes_comp -> operateur ));
	    xmlSetProp ( node_3,
			 "Txt",
			 textes_comp -> texte );
	    xmlSetProp ( node_3,
			 "Util_txt",
			 itoa ( textes_comp -> utilise_txt ));
	    xmlSetProp ( node_3,
			 "Comp_1",
			 itoa ( textes_comp -> comparateur_1 ));
	    xmlSetProp ( node_3,
			 "Lien_1_2",
			 itoa ( textes_comp -> lien_1_2 ));
	    xmlSetProp ( node_3,
			 "Comp_2",
			 itoa ( textes_comp -> comparateur_2 ));
	    xmlSetProp ( node_3,
			 "Mont_1",
			 itoa ( textes_comp -> montant_1 ));
	    xmlSetProp ( node_3,
			 "Mont_2",
			 itoa ( textes_comp -> montant_2 ));
	    pointeur_liste = pointeur_liste -> next;
	}



	xmlNewTextChild ( node_etat,
			  NULL,
			  "Montant",
			  itoa ( etat -> utilise_montant ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Montant_devise",
			  itoa ( etat -> choix_devise_montant ));

	node_2 = xmlNewChild ( node_etat,
			       NULL,
			       "Montant_comp",
			       NULL );

	pointeur_liste = etat -> liste_struct_comparaison_montants;

	while ( pointeur_liste )
	{
	    struct struct_comparaison_montants_etat *montants_comp;
	    xmlNodePtr node_3;

	    montants_comp = pointeur_liste -> data;

	    node_3 = xmlNewChild ( node_2,
				   NULL,
				   "Comp",
				   NULL );

	    xmlSetProp ( node_3,
			 "Lien_struct",
			 itoa ( montants_comp -> lien_struct_precedente ));
	    xmlSetProp ( node_3,
			 "Comp_1",
			 itoa ( montants_comp -> comparateur_1 ));
	    xmlSetProp ( node_3,
			 "Lien_1_2",
			 itoa ( montants_comp -> lien_1_2 ));
	    xmlSetProp ( node_3,
			 "Comp_2",
			 itoa ( montants_comp -> comparateur_2 ));
	    xmlSetProp ( node_3,
			 "Mont_1",
			 g_strdup_printf ( "%4.7f",
					   montants_comp -> montant_1 ));
	    xmlSetProp ( node_3,
			 "Mont_2",
			 g_strdup_printf ( "%4.7f",
					   montants_comp -> montant_2 ));
	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Excl_nul",
			  itoa ( etat -> exclure_montants_nuls ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_mod_paie",
			  itoa ( etat -> utilise_mode_paiement ));

	node_2 = xmlNewChild ( node_etat,
			       NULL,
			       "Liste_mod_paie",
			       NULL );

	pointeur_liste = etat -> noms_modes_paiement;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    xmlNodePtr node_3;

	    node_3 = xmlNewChild ( node_2,
				   NULL,
				   "Mode_paie",
				   NULL );

	    xmlSetProp ( node_3,
			 "Nom",
			 pointeur_liste -> data );

	    pointeur_liste = pointeur_liste -> next;
	}

	pointeur_liste_2 = pointeur_liste_2 -> next;
    }




    /* l'arbre est fait, on sauvegarde */
#ifndef _WIN32
    xmlIndentTreeOutput = 1;
#endif
    resultat = xmlSaveFormatFile ( nouveau_fichier, doc, 1 );

    /* on libère la memoire */

    xmlFreeDoc ( doc );

    if ( resultat == -1 )
    {
	dialogue_error ( g_strdup_printf ( _("Cannot save file '%s': %s"),
					   nouveau_fichier, latin2utf8(strerror(errno)) ));
	return ( FALSE );
    }


    /* si c'est un nouveau fichier, on met à 600 ses permissions */

    if ( mettre_permission )
	chmod ( nouveau_fichier,
		S_IRUSR | S_IWUSR );


    etat.en_train_de_sauvegarder = 0;

    return ( TRUE );
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* crée ou supprime un fichier du nom .nom.swp */
/* renvoie true si ok */
/***********************************************************************************************************/

gboolean modification_etat_ouverture_fichier ( gboolean fichier_ouvert )
{
    struct stat buffer_stat;
    int result;
    gchar *nom_fichier_lock;
    gchar **tab_str;
    gint i;

    /*     on efface et on recommence... bon, changement de technique : lors de l'ouverture */
    /* 	d'un fichier, on crée un fichier .nom.swp qu'on efface à sa fermeture */

    /*     si on ne force pas l'enregistrement et si le fichier était déjà ouvert, on ne fait rien */

    if ( (etat.fichier_deja_ouvert
	  &&
	  !etat.force_enregistrement)
	 ||
	 !nom_fichier_comptes ||
	 !nom_fichier_comptes ||
	 !strlen(nom_fichier_comptes) )
	return TRUE;

    /*     on commence par vérifier que le fichier de nom_fichier_comptes existe bien */

    result = stat ( nom_fichier_comptes, &buffer_stat);

    if ( result == -1 )
    {
	dialogue_error (g_strdup_printf (_("Cannot open file '%s' to mark it as used: %s"),
					 nom_fichier_comptes,
					 latin2utf8 (strerror(errno))));
	return FALSE;
    }


    /*     création du nom du fichier swp */

    tab_str = g_strsplit ( nom_fichier_comptes,
			   "/",
			   0 );

    i=0;

    while ( tab_str[i+1] )
	i++;

    tab_str[i] = g_strconcat ( ".",
			       tab_str[i],
			       ".swp",
			       NULL );
    nom_fichier_lock = g_strjoinv ( "/",
				   tab_str );
    g_strfreev ( tab_str );

    /*     maintenant on sépare entre l'effacement ou la création du fichier swp */

    if ( fichier_ouvert )
    {
	/* 	on ouvre le fichier, donc on crée le fichier de lock */

	FILE *fichier;

	/* 	commence par tester si ce fichier existe, si c'est le cas on prévient l'utilisateur */
	/* 	    avec possibilité d'annuler l'action ou d'effacer le fichier de lock */

	result = stat ( nom_fichier_lock, &buffer_stat);

	if ( result != -1 )
	{
	    /* 	    le fichier de lock existe */

	    dialogue_conditional_hint ( g_strdup_printf( _("File \"%s\" is already opened"),
							 nom_fichier_comptes),
					_("Either this file is already opened by another user or it wasn't closed correctly (maybe Grisbi crashed?).\nGrisbi can't save the file unless you activate the \"Force saving locked files\" option in setup."),
					&(etat.display_message_lock_active) );
	    
	    /* 	    on retourne true, vu que le fichier est déjà créé et qu'on a prévenu */

	    etat.fichier_deja_ouvert = 1;
	    return TRUE;
	}

	etat.fichier_deja_ouvert = 0;

	fichier = fopen ( nom_fichier_lock,
			  "w" );

	if ( !fichier )
	{
	    dialogue_error (g_strdup_printf (_("Cannot write lock file :'%s': %s"),
					     nom_fichier_comptes,
					     latin2utf8 (strerror(errno))));
	    return FALSE;
	}

	fclose ( fichier );
	return TRUE;
    }
    else
    {
	/* 	on ferme le fichier, donc on détruit le fichier de lock */

	etat.fichier_deja_ouvert = 0;

	/* 	on vérifie d'abord que ce fichier existe */

	result = stat ( nom_fichier_lock, &buffer_stat);

	if ( result == -1 )
	{
	    /* 	    le fichier de lock n'existe */
	    /* 	    on s'en fout, de toute façon fallait le virer, on s'en va */

	    return TRUE;
	}

	result = remove ( nom_fichier_lock );

	if ( result == -1 )
	{
	    dialogue_error (g_strdup_printf (_("Cannot erase lock file :'%s': %s"),
					     nom_fichier_comptes,
					     latin2utf8 (strerror(errno))));
	    return FALSE;
	}
	return TRUE;
    }
}
/***********************************************************************************************************/




/***********************************************************************************************************/
gboolean enregistre_categ ( gchar *nom_categ )
{
    xmlDocPtr doc;
    xmlNodePtr node;
    gint resultat;
    GSList *pointeur_liste;

    /* creation de l'arbre xml en memoire */

    doc = xmlNewDoc("1.0");

    /* la racine est grisbi */

    doc->children = xmlNewDocNode ( doc,
				    NULL,
				    "Grisbi_categ",
				    NULL );

    /* on commence à ajouter les generalites */

    node = xmlNewChild ( doc->children,
			 NULL,
			 "Generalites",
			 NULL );
    xmlNewTextChild ( node,
		      NULL,
		      "Version_fichier_categ",
		      VERSION_FICHIER_CATEG );

    xmlNewTextChild ( node,
		      NULL,
		      "Version_grisbi",
		      VERSION );


    node = xmlNewChild ( doc->children,
			 NULL,
			 "Details_des_categories",
			 NULL );


    pointeur_liste = liste_struct_categories;

    while ( pointeur_liste )
    {
	struct struct_categ *categ;
	xmlNodePtr node_categ;
	GSList *pointeur_sous_categ;

	categ = pointeur_liste -> data;


	node_categ = xmlNewChild ( node,
				   NULL,
				   "Categorie",
				   NULL );

	xmlSetProp ( node_categ,
		     "Nom",
		     categ -> nom_categ );

	xmlSetProp ( node_categ,
		     "Type",
		     itoa ( categ -> type_categ ));

	xmlSetProp ( node_categ,
		     "No_derniere_sous_cagegorie",
		     itoa ( categ -> no_derniere_sous_categ ));


	/* on ajoute les sous categories */

	pointeur_sous_categ = categ -> liste_sous_categ;

	while ( pointeur_sous_categ )
	{
	    struct struct_sous_categ *sous_categ;
	    xmlNodePtr node_sous_categ;

	    sous_categ = pointeur_sous_categ -> data;

	    node_sous_categ = xmlNewChild ( node_categ,
					    NULL,
					    "Sous-categorie",
					    NULL );

	    xmlSetProp ( node_sous_categ,
			 "No",
			 itoa ( sous_categ -> no_sous_categ ));

	    xmlSetProp ( node_sous_categ,
			 "Nom",
			 sous_categ -> nom_sous_categ );

	    pointeur_sous_categ = pointeur_sous_categ -> next;
	}
	pointeur_liste = pointeur_liste -> next;
    }


    /* l'arbre est fait, on sauvegarde */

    resultat = xmlSaveFile ( nom_categ,
			     doc );

    /* on libÃšre la memoire */

    xmlFreeDoc ( doc );


    if ( resultat == -1 )
    {
	dialogue_error ( g_strdup_printf ( _("Cannot save file '%s': %s"),
					   nom_categ,
					   latin2utf8 (strerror ( errno ) )));
	return ( FALSE );
    }


    return ( TRUE );
}
/***********************************************************************************************************/







/***********************************************************************************************************/
gboolean charge_categ ( gchar *nom_categ )
{
    xmlDocPtr doc;

    doc = xmlParseFile ( nom_categ );

    if ( doc )
    {
	/* vérifications d'usage */

	xmlNodePtr root = xmlDocGetRootElement(doc);

	if ( !doc->children
	     ||
	     !doc->children->name
	     ||
	     g_strcasecmp ( doc->children->name,
			    "Grisbi_categ" ))
	{
	    dialogue_error ( _("This file is not a Grisbi category list file") );
	    xmlFreeDoc ( doc );
	    return ( FALSE );
	}

	/* récupère la version de fichier */
	if (( strcmp (  xmlNodeGetContent ( root->children->next->children->next ), VERSION_FICHIER_CATEG )))
	  {
	    dialogue_warning_hint ( g_strdup_printf (_("This budgetary lines list has been produced with grisbi version %s, Grisbi will nevertheless try to import it."), xmlNodeGetContent ( root->children->next->children->next )),
				    _("Version mismatch") );
	  }

	if (( !strcmp (  xmlNodeGetContent ( root->children->next->children->next ),
			 "0.4.0" )))
	    return ( charge_categ_version_0_4_0 ( doc ));
	if (( !strcmp (  xmlNodeGetContent ( root->children->next->children->next ),
			 "0.4.1" )))
	    return ( charge_categ_version_0_4_0 ( doc ));
	if (( !strcmp (  xmlNodeGetContent ( root->children->next->children->next ),
			 "0.5.0" )))
	    return ( charge_categ_version_0_4_0 ( doc ));

	/* 	à ce niveau, c'est que que la version n'est pas connue de grisbi, on donne alors */
	/* la version nécessaire pour l'ouvrir */

	dialogue_error ( g_strdup_printf ( _("Grisbi version %s is needed to open this file"),
					   xmlNodeGetContent ( doc->children->children->children->next )));

	xmlFreeDoc ( doc );

	return ( FALSE );
    }
    else
    {
	dialogue_error ( _("Invalid categories file") );
	return ( FALSE );
    }
}
/***********************************************************************************************************/




/***********************************************************************************************************/
gboolean charge_categ_version_0_4_0 ( xmlDocPtr doc )
{
    xmlNodePtr node;
    struct struct_categ *categ;

    categ = calloc ( 1,
		     sizeof ( struct struct_categ ));

    /* on place node sur les generalites */

    node = xmlDocGetRootElement(doc) -> children;


    while ( node )
    {
	if ( !strcmp ( node -> name,
		       "Generalites" ) )
	{
	    xmlNodePtr node_generalites;

	    /* node_generalites va faire le tour des generalites */

	    node_generalites = node -> children;

	    while ( node_generalites )
	    {
		/* rien pour l'instant, peut être un jour ? */

		node_generalites = node_generalites -> next;
	    }
	}

	/* on recupère ici les catég */

	if ( !strcmp ( node -> name,
		       "Details_des_categories" ))
	{
	    xmlNodePtr node_detail;
	    xmlNodePtr node_sous_categ;

	    node_detail = node -> children;

	    while ( node_detail )
	    {
		GSList *liste_tmp;
		struct struct_categ *categorie;

		if ( node_detail -> type != XML_TEXT_NODE )
		{

		    /* 	      on doit réaliser une fusion, pour ça, soit la catég existe, et on fait le */
		    /* tour des sous catég en ajoutant celles qui n'existent pas, soit elle n'existe pas et on */
		    /* ajoute la catég et ses sous categ */

		    liste_tmp = g_slist_find_custom ( liste_struct_categories,
						      xmlGetProp ( node_detail,
								   "Nom" ),
						      (GCompareFunc) recherche_categorie_par_nom );

		    if ( liste_tmp )
		    {
			/* 		  la catégorie existe, on fait le tour des sous catégories */

			categorie = liste_tmp -> data;

			node_sous_categ = node_detail -> children;

			while ( node_sous_categ )
			{
			    struct struct_sous_categ *sous_categ;
			    GSList *liste_tmp_2;

			    if ( node_sous_categ -> type != XML_TEXT_NODE )
			    {
				/* on ne prend la sous catég que si elle n'existe pas */

				liste_tmp_2 = g_slist_find_custom ( categorie -> liste_sous_categ,
								    xmlGetProp ( node_sous_categ,
										 "Nom" ),
								    (GCompareFunc) recherche_sous_categorie_par_nom );

				if ( !liste_tmp_2 )
				{

				    sous_categ = calloc ( 1,
							  sizeof ( struct struct_sous_categ ) );

				    sous_categ -> no_sous_categ = ++categorie -> no_derniere_sous_categ;

				    sous_categ -> nom_sous_categ = xmlGetProp ( node_sous_categ,
										"Nom" );

				    categorie -> liste_sous_categ = g_slist_append ( categorie -> liste_sous_categ,
										     sous_categ );
				}
			    }
			    node_sous_categ = node_sous_categ -> next;
			}
		    }
		    else
		    {
			/* la catégorie n'existe pas, on l'ajoute */


			categorie = calloc ( 1,
					     sizeof ( struct struct_categ ) );

			categorie -> no_categ = ++no_derniere_categorie;
			nb_enregistrements_categories++;

			categorie -> nom_categ = xmlGetProp ( node_detail,
							      "Nom" );
			categorie -> type_categ = my_atoi ( xmlGetProp ( node_detail,
								      "Type" ));
			categorie -> no_derniere_sous_categ = my_atoi ( xmlGetProp ( node_detail,
										  "No_derniere_sous_cagegorie" ));

			/*  pour chaque categorie, on recupère les sous-categories */

			categorie -> liste_sous_categ = NULL;
			node_sous_categ = node_detail -> children;

			while ( node_sous_categ )
			{

			    if ( node_sous_categ -> type != XML_TEXT_NODE )
			    {
				struct struct_sous_categ *sous_categ;

				sous_categ = calloc ( 1,
						      sizeof ( struct struct_sous_categ ) );

				sous_categ -> no_sous_categ = my_atoi ( xmlGetProp ( node_sous_categ,
										     "No" ));
				sous_categ -> nom_sous_categ = xmlGetProp ( node_sous_categ,
									    "Nom" );

				categorie -> liste_sous_categ = g_slist_append ( categorie -> liste_sous_categ,
										 sous_categ );
			    }
			    node_sous_categ = node_sous_categ -> next;
			}

			liste_struct_categories = g_slist_append ( liste_struct_categories,
								   categorie );
		    }
		}
		node_detail = node_detail -> next;
	    }
	}
	node = node -> next;
    }

    /* on libère la memoire */

    xmlFreeDoc ( doc );

    /* creation de la liste des categ pour le combofix */

    creation_liste_categ_combofix ();
    remplit_arbre_categ ();

    /* on rafraichit la liste des categ */


    modification_fichier ( TRUE );

    return ( TRUE );
}
/***********************************************************************************************************/








/***********************************************************************************************************/
gboolean enregistre_ib ( gchar *nom_ib )
{
    xmlDocPtr doc;
    xmlNodePtr node;
    gint resultat;
    GSList *pointeur_liste;

    /* creation de l'arbre xml en memoire */

    doc = xmlNewDoc("1.0");

    /* la racine est grisbi */

    doc->children = xmlNewDocNode ( doc,
				    NULL,
				    "Grisbi_ib",
				    NULL );

    /* on commence à ajouter les generalites */

    node = xmlNewChild ( doc->children,
			 NULL,
			 "Generalites",
			 NULL );
    xmlNewTextChild ( node,
		      NULL,
		      "Version_fichier_ib",
		      VERSION_FICHIER_IB );

    xmlNewTextChild ( node,
		      NULL,
		      "Version_grisbi",
		      VERSION );


    node = xmlNewChild ( doc->children,
			 NULL,
			 "Details_des_ib",
			 NULL );


    pointeur_liste = liste_struct_imputation;

    while ( pointeur_liste )
    {
	struct struct_imputation *ib;
	xmlNodePtr node_ib;
	GSList *pointeur_sous_ib;

	ib = pointeur_liste -> data;


	node_ib = xmlNewChild ( node,
				NULL,
				"Imputation",
				NULL );

	xmlSetProp ( node_ib,
		     "Nom",
		     ib -> nom_imputation );

	xmlSetProp ( node_ib,
		     "Type",
		     itoa ( ib -> type_imputation ));

	xmlSetProp ( node_ib,
		     "No_derniere_sous_imputation",
		     itoa ( ib -> no_derniere_sous_imputation ));


	/* on ajoute les sous ib */

	pointeur_sous_ib = ib -> liste_sous_imputation;

	while ( pointeur_sous_ib )
	{
	    struct struct_sous_imputation *sous_ib;
	    xmlNodePtr node_sous_ib;

	    sous_ib = pointeur_sous_ib -> data;

	    node_sous_ib = xmlNewChild ( node_ib,
					 NULL,
					 "Sous-imputation",
					 NULL );

	    xmlSetProp ( node_sous_ib,
			 "No",
			 itoa ( sous_ib -> no_sous_imputation ));

	    xmlSetProp ( node_sous_ib,
			 "Nom",
			 sous_ib -> nom_sous_imputation );

	    pointeur_sous_ib = pointeur_sous_ib -> next;
	}
	pointeur_liste = pointeur_liste -> next;
    }


    /* l'arbre est fait, on sauvegarde */

    resultat = xmlSaveFormatFile ( nom_ib,
				   doc,
				   1 );

    /* on libère la memoire */

    xmlFreeDoc ( doc );


    if ( resultat == -1 )
    {
	dialogue_error ( g_strconcat ( _("Cannot save file '%s': %s"),
				       nom_ib, latin2utf8 (strerror ( errno )) ));
	return ( FALSE );
    }


    return ( TRUE );
}
/***********************************************************************************************************/




/***********************************************************************************************************/
gboolean charge_ib ( gchar *nom_ib )
{
    xmlDocPtr doc;

    doc = xmlParseFile ( nom_ib );

    if ( doc )
    {
	/* vérifications d'usage */

	xmlNodePtr root = xmlDocGetRootElement(doc);
	    
	if ( !doc->children
	     ||
	     !doc->children->name
	     ||
	     g_strcasecmp ( doc->children->name,
			    "Grisbi_ib" ))
	{
	    dialogue_error ( _("This file is not a grisbi budgetary lines file") );
	    xmlFreeDoc ( doc );
	    return ( FALSE );
	}

	/* récupère la version de fichier */

	if (( !strcmp (  xmlNodeGetContent ( root->children->next->children->next ),
			 "0.4.0" )))
	    return ( charge_ib_version_0_4_0 ( doc ));
	if (( !strcmp (  xmlNodeGetContent ( root->children->next->children->next ),
			 "0.4.1" )))
	    return ( charge_ib_version_0_4_0 ( doc ));
	if (( !strcmp (  xmlNodeGetContent ( root->children->next->children->next ),
			 "0.5.0" )))
	    return ( charge_ib_version_0_4_0 ( doc ));

	/* 	à ce niveau, c'est que que la version n'est pas connue de grisbi, on donne alors */
	/* la version nÃ©cessaire pour l'ouvrir */

	if (( strcmp (  xmlNodeGetContent ( root->children->next->children->next ), VERSION_FICHIER_IB )))
	  {
	    dialogue_warning_hint ( g_strdup_printf (_("This budgetary lines list has been produced with grisbi version %s, Grisbi will nevertheless try to import it."), xmlNodeGetContent ( root->children->next->children->next )),
				    _("Version mismatch") );
	  }

	return ( charge_ib_version_0_4_0 ( doc ));
    }
    else
    {
	dialogue_error ( _("Invalid budgetary lines file") );
	return ( FALSE );
    }
}
/***********************************************************************************************************/




/***********************************************************************************************************/
gboolean charge_ib_version_0_4_0 ( xmlDocPtr doc )
{
    xmlNodePtr node;
    struct struct_imputation *ib;
    xmlNodePtr root = xmlDocGetRootElement(doc);

    ib = calloc ( 1,
		  sizeof ( struct struct_imputation ));

    /* on place node sur les generalites */

    node = root -> children;


    while ( node )
    {
	if ( !strcmp ( node -> name,
		       "Generalites" ) )
	{
	    xmlNodePtr node_generalites;

	    /* node_generalites va faire le tour des generalites */

	    node_generalites = node -> children;

	    while ( node_generalites )
	    {
		/* rien pour l'instant, peut être un jour ? */


		node_generalites = node_generalites -> next;
	    }
	}

	/* on recupère ici les ib */

	if ( !strcmp ( node -> name,
		       "Details_des_ib" ))
	{
	    xmlNodePtr node_detail;
	    xmlNodePtr node_sous_ib;

	    node_detail = node -> children;

	    while ( node_detail )
	    {
		GSList *liste_tmp;
		struct struct_imputation *ib;

		if ( node_detail -> type != XML_TEXT_NODE )
		{

		    /* 	      on doit réaliser une fusion, pour ça, soit l'ib existe, et on fait le */
		    /* tour des sous ib en ajoutant celles qui n'existent pas, soit elle n'existe pas et on */
		    /* ajoute l'ib et ses sous ib */

		    liste_tmp = g_slist_find_custom ( liste_struct_imputation,
						      latin2utf8(xmlGetProp ( node_detail,
									      "Nom" )),
						      (GCompareFunc) recherche_imputation_par_nom );


		    if ( liste_tmp )
		    {
			/* 		  la catégorie existe, on fait le tour des sous catégories */

			ib = liste_tmp -> data;

			node_sous_ib = node_detail -> children;

			while ( node_sous_ib )
			{
			    struct struct_sous_imputation *sous_ib;
			    GSList *liste_tmp_2;

			    if ( node_sous_ib -> type != XML_TEXT_NODE )
			    {
				/* on ne prend la sous catég que si elle n'existe pas */

				liste_tmp_2 = g_slist_find_custom ( ib -> liste_sous_imputation,
								    latin2utf8(xmlGetProp ( node_sous_ib,
											    "Nom" )),
								    (GCompareFunc) recherche_sous_imputation_par_nom );

				if ( !liste_tmp_2 )
				{

				    sous_ib = calloc ( 1,
						       sizeof ( struct struct_sous_imputation ) );

				    sous_ib -> no_sous_imputation = ++ib -> no_derniere_sous_imputation;

				    sous_ib -> nom_sous_imputation = latin2utf8(xmlGetProp ( node_sous_ib,
											     "Nom" ));

				    ib -> liste_sous_imputation = g_slist_append ( ib -> liste_sous_imputation,
										   sous_ib );
				}
			    }
			    node_sous_ib = node_sous_ib -> next;
			}
		    }
		    else
		    {
			/* l'ib n'existe pas, on l'ajoute */


			ib = calloc ( 1,
				      sizeof ( struct struct_imputation ) );

			ib -> no_imputation = ++no_derniere_imputation;
			nb_enregistrements_imputations++;

			ib -> nom_imputation = latin2utf8(xmlGetProp ( node_detail,
								       "Nom" ));
			ib -> type_imputation = my_atoi ( latin2utf8(xmlGetProp ( node_detail,
									       "Type" )));
			ib -> no_derniere_sous_imputation = my_atoi ( latin2utf8(xmlGetProp ( node_detail,
											   "No_derniere_sous_imputation" )));

			/*  pour chaque ib, on recupère les sous-ib */

			ib -> liste_sous_imputation = NULL;
			node_sous_ib = node_detail -> children;

			while ( node_sous_ib )
			{
			    if ( node_sous_ib -> type != XML_TEXT_NODE )
			    {
				struct struct_sous_imputation *sous_ib;

				sous_ib = calloc ( 1,
						   sizeof ( struct struct_sous_imputation ) );

				sous_ib -> no_sous_imputation = my_atoi ( latin2utf8(xmlGetProp ( node_sous_ib,
												  "No" )));
				sous_ib -> nom_sous_imputation = latin2utf8(xmlGetProp ( node_sous_ib,
											 "Nom" ));

				ib -> liste_sous_imputation = g_slist_append ( ib -> liste_sous_imputation,
									       sous_ib );
			    }
			    node_sous_ib = node_sous_ib -> next;
			}

			liste_struct_imputation = g_slist_append ( liste_struct_imputation,
								   ib );
		    }
		}
		node_detail = node_detail -> next;
	    }
	}
	node = node -> next;
    }

    /* on libère la memoire */

    xmlFreeDoc ( doc );

    /* creation de la liste des ib pour le combofix */

    creation_liste_imputation_combofix ();
    remplit_arbre_imputation ();


    modification_fichier ( TRUE );

    return ( TRUE );
}
/***********************************************************************************************************/




/***********************************************************************************************************/
void propose_changement_permissions ( void )
{
    GtkWidget *dialog, *vbox, *checkbox;
    gint resultat;

    dialog = gtk_message_dialog_new ( GTK_WINDOW ( window ),
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_QUESTION,
				      GTK_BUTTONS_YES_NO,
				      _("Your account file should not be readable by anybody else, but it is. You should change its permissions.\nShould this be fixed now?") );

    vbox = GTK_DIALOG(dialog) -> vbox;
    checkbox = new_checkbox_with_title ( _("Do not show this message again"),
					 &(etat.display_message_file_readable), NULL);
    gtk_box_pack_start ( GTK_BOX ( vbox ), checkbox, FALSE, FALSE, 6 );
    gtk_widget_show_all ( dialog );

    resultat = gtk_dialog_run ( GTK_DIALOG(dialog) );

    if ( resultat == GTK_RESPONSE_YES )
    {
	chmod ( nom_fichier_comptes, S_IRUSR | S_IWUSR );
    }

    gtk_widget_destroy ( dialog );
}
/***********************************************************************************************************/



