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
#include "structures.h"
#include "variables-extern.c"
#include "fichiers_io.h"


#include "categories_onglet.h"
#include "dialog.h"
#include "imputation_budgetaire.h"
#include "menu.h"
#include "operations_liste.h"
#include "search_glist.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_files.h"



extern gint valeur_echelle_recherche_date_import;
extern gint affichage_echeances; 
extern gint affichage_echeances_perso_nb_libre; 
extern gint affichage_echeances_perso_j_m_a; 
extern gint nb_echeances;
extern gint no_derniere_echeance;
extern GSList *gsliste_echeances; 

/****************************************************************************/
void remove_file_from_last_opened_files_list ( gchar * nom_fichier )
{
    gint i, j;

    efface_derniers_fichiers_ouverts();

    for ( i = 0 ; i < nb_derniers_fichiers_ouverts ; i++ )
    {
	if ( ! strcmp (nom_fichier_comptes, tab_noms_derniers_fichiers_ouverts[i]) )
	{
	    for ( j = i; j < nb_derniers_fichiers_ouverts-1; j++ )
	    {
		tab_noms_derniers_fichiers_ouverts[j] = tab_noms_derniers_fichiers_ouverts[j+1];

	    }
	    break;
	}
    }
    nb_derniers_fichiers_ouverts--;
    affiche_derniers_fichiers_ouverts();
}
/****************************************************************************/



/****************************************************************************/
/** Procédure qui charge les opérations en mémoire sous forme de structures**/
/** elle rend la main en ayant initialisée la variable p_tab_nom_de_compte,**/
/** tableau de pointeurs vers chaque compte :                              **/
/****************************************************************************/

gboolean charge_operations ( void )
{
    struct stat buffer_stat;
    xmlDocPtr doc;
    int result;

    /* vérification de la permission du fichier */

    result = utf8_stat ( nom_fichier_comptes, &buffer_stat);

    /* check here if it's not a regular file */

    if ( !S_ISREG (buffer_stat.st_mode))
    {
	dialogue_error ( g_strdup_printf ( _("%s doen't seem to be a regular file,\nplease check it and try again."),
					   nom_fichier_comptes ));
	return ( FALSE );
	}

    if ( result != -1
	 && 
	 buffer_stat.st_mode != 33152
	 &&
	 !etat.display_message_file_readable )
	propose_changement_permissions();


    /* on commence par ouvrir le fichier en xml */

    if ( result != -1 )
    {
	doc = utf8_xmlParseFile ( nom_fichier_comptes );

	if ( doc )
	{
	    /* vérifications d'usage */
	    xmlNodePtr root = xmlDocGetRootElement(doc);

	    if ( !root
		 ||
		 !root->name
		 ||
		 g_strcasecmp ( root->name,
				"Grisbi" ))
	    {
		dialogue_error ( _("Invalid accounts file") );
		xmlFreeDoc ( doc );
		return ( FALSE );
	    }

	    /* récupère la version de fichier */

	    if (( !strcmp (  xmlNodeGetContent ( root->children->next->children->next ),
			     "0.3.2" )))
		return ( charge_operations_version_0_3_2 ( doc ));

	    if (( !strcmp (  xmlNodeGetContent ( root->children->next->children->next ),
			     "0.3.3" )))
		return ( charge_operations_version_0_3_2 ( doc ));

	    if (( !strcmp (  xmlNodeGetContent ( root->children->next->children->next ),
			     "0.4.0" )))
		return ( charge_operations_version_0_4_0 ( doc ));

	    if (( !strcmp (  xmlNodeGetContent ( root->children->next->children->next ),
			     "0.4.1" )))
		return ( charge_operations_version_0_4_1 ( doc ));
	    if (( !strncmp (  xmlNodeGetContent ( root->children->next->children->next ),
			      "0.5.0", 5 )))
		return ( charge_operations_version_0_5_0 ( doc ));

	    /* 	à ce niveau, c'est que que la version n'est pas connue de grisbi, on donne alors */
	    /* la version nécessaire pour l'ouvrir */

	    dialogue_error ( g_strdup_printf ( _("Grisbi version %s is needed to open this file"),
					       xmlNodeGetContent ( root->children->next -> children->next  )));

	    xmlFreeDoc ( doc );

	    return ( FALSE );
	}

	if ( errno )
	    dialogue_error (g_strdup_printf (_("Cannot open file '%s': %s"), nom_fichier_comptes,
					     latin2utf8 (strerror(errno))));
	else
	    dialogue_error (g_strdup_printf (_("Cannot open file '%s' : this is not a grisbi file"), nom_fichier_comptes ));

	remove_file_from_last_opened_files_list ( nom_fichier_comptes );
	return ( FALSE );
    }
    else
    {
	remove_file_from_last_opened_files_list ( nom_fichier_comptes );
	dialogue_error (g_strdup_printf (_("Cannot open file '%s': %s"), nom_fichier_comptes,
					 latin2utf8 (strerror(errno))));
	return FALSE;
    }
}
/***********************************************************************************************************/





/*****************************************************************************/
/* ajout de la 0.3.2 et 0.3.3 */
/*****************************************************************************/

gboolean charge_operations_version_0_3_2 ( xmlDocPtr doc )
{
    xmlNodePtr node_1;
    gint i;
    gchar *nom_sauvegarde;
    xmlNodePtr root = xmlDocGetRootElement(doc);

    etat.en_train_de_charger = 1;

    /* on copie le fichier en ajoutant l'extension 0_3_3 et on met message d'avertissement */

    nom_sauvegarde = g_strconcat ( nom_fichier_comptes,
				   "_version_0_3_3",
				   NULL );

    /* FIXME: potential security problems there !!! */
    system ( g_strdup_printf ( "cp %s %s",
			       nom_fichier_comptes,
			       nom_sauvegarde ));
    dialogue ( g_strdup_printf ( _("Warning: Grisbi data format has changed. Grisbi made a backup under '%s'.\nKeep it for a while just in case."),
				 nom_sauvegarde ));

    /* on place node_1 sur les généralités */

    node_1 = root -> children;

    /*   on met en place la boucle de node_1, qui va successivement passer */
    /*     par les généralités, les comptes, les échéances ... */

    while ( node_1 )
    {

	/* on récupère ici les généralités */

	if ( !strcmp ( node_1 -> name,
		       "Généralités" ) )
	{
	    xmlNodePtr node_generalites;

	    /* node_generalites va faire le tour des généralités */

	    node_generalites = node_1 -> children;

	    while ( node_generalites )
	    {
		/* FIXME: One day, put a non-word there.  Rationale:
		   i10nizing this would break things and using French or
		   even English is not an option for foreigners.  Btw,
		   we should not rely on a French oritented file
		   format.  [benj] */
		if ( !strcmp ( node_generalites -> name,
			       "Fichier_ouvert" ))
		    if ( (etat.fichier_deja_ouvert  = my_atoi ( xmlNodeGetContent ( node_generalites ))))
		    {
			dialogue_conditional_hint ( g_strdup_printf( _("File \"%s\" is already opened"), nom_fichier_comptes),
						    _("Either this file is already opened by another user or it wasn't closed correctly (maybe Grisbi crashed?).\nGrisbi can't save the file unless you activate the \"Force saving locked files\" option in setup."), &(etat.display_message_lock_active) );
		    }	    

		if ( !strcmp ( node_generalites -> name,
			       "Backup" ))
		    nom_fichier_backup = xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Titre" ))
		    titre_fichier = xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Adresse_commune" ))
		    adresse_commune	= xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_exercices" ))
		    etat.utilise_exercice = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_IB" ))
		    etat.utilise_imputation_budgetaire = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_PC" ))
		    etat.utilise_piece_comptable = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_info_BG" ))
		    etat.utilise_info_banque_guichet = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Numéro_devise_totaux_tiers" ))
		    no_devise_totaux_tiers = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Type_affichage_des_échéances" ))
		    affichage_echeances = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Affichage_echeances_perso_nb_libre" ))
		    affichage_echeances_perso_nb_libre = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Type_affichage_perso_echeances" ))
		    affichage_echeances_perso_j_m_a = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Numéro_derniere_operation" ))
		    no_derniere_operation= my_atoi ( xmlNodeGetContent ( node_generalites ));

		node_generalites = node_generalites -> next;
	    }
	}

	/* on récupère ici les comptes et opérations */

	if ( !strcmp ( node_1 -> name,
		       "Comptes" ))
	{
	    xmlNodePtr node_comptes;

	    /* node_comptes va faire le tour de l'arborescence des comptes */

	    node_comptes = node_1 -> children;

	    while ( node_comptes )
	    {

		/* on va récupérer ici les généralités des comptes */

		if ( !strcmp ( node_comptes -> name,
			       "Généralités" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_comptes -> children;

		    while ( node_generalites )
		    {
			/* récupère l'ordre des comptes */

			if ( !strcmp ( node_generalites -> name,
				       "Ordre_des_comptes" ))
			{
			    gchar **pointeur_char;

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

			    /* Création du tableau de pointeur vers les structures de comptes */

			    p_tab_nom_de_compte = malloc ( nb_comptes * sizeof ( gpointer ));
			    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
			}

			/* récupère le compte courant */

			if ( !strcmp ( node_generalites -> name,
				       "Compte_courant" ))
			{
			    compte_courant = my_atoi ( xmlNodeGetContent ( node_generalites ));
			    p_tab_nom_de_compte_courant = p_tab_nom_de_compte + compte_courant;
			}

			node_generalites = node_generalites -> next;
		    }
		}

		/* on récupère les détails de chaque compte */

		if ( !strcmp ( node_comptes -> name,
			       "Compte" ))
		{
		    xmlNodePtr node_nom_comptes;

		    /* normalement p_tab_nom_de_compte_variable est déjà placé */

		    /* on crée la structure du compte */

		    *p_tab_nom_de_compte_variable = calloc ( 1,
							     sizeof (struct donnees_compte));

		    /* les valeurs AFFICHAGE_R et NB_LIGNES_OPE par défaut */

		    AFFICHAGE_R = 0;
		    NB_LIGNES_OPE = 3;

		    /* on fait le tour dans l'arbre nom, cad : les détails, détails de type et détails des opérations */

		    node_nom_comptes = node_comptes -> children;

		    while ( node_nom_comptes )
		    {
			/* on récupère les détails du compte */

			if ( !strcmp ( node_nom_comptes -> name,
				       "Détails" ))
			{
			    xmlNodePtr node_detail;

			    node_detail = node_nom_comptes -> children;

			    while ( node_detail )
			    {

				if ( !strcmp ( node_detail -> name,
					       "Nom" ))
				    NOM_DU_COMPTE = xmlNodeGetContent ( node_detail );

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
					       "Nb_opérations" ))
				    NB_OPE_COMPTE = my_atoi ( xmlNodeGetContent ( node_detail ));

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
					       "Clé_du_compte" ))
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
					       "Solde_mini_autorisé" ))
				    SOLDE_MINI = my_strtod ( xmlNodeGetContent ( node_detail ),
							     NULL );

				if ( !strcmp ( node_detail -> name,
					       "Solde_courant" ))
				    SOLDE_COURANT = my_strtod ( xmlNodeGetContent ( node_detail ),
								NULL );

				if ( !strcmp ( node_detail -> name,
					       "Date_dernier_relevé" ))
				{
				    gchar **pointeur_char;

				    if ( xmlNodeGetContent ( node_detail ))
				    {
					pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail ),
								     "/",
								     3 );
					DATE_DERNIER_RELEVE = g_date_new_dmy ( my_atoi ( pointeur_char [0] ),
									       my_atoi ( pointeur_char [1] ),
									       my_atoi ( pointeur_char [2] ));
					g_strfreev ( pointeur_char );
				    }
				}

				if ( !strcmp ( node_detail -> name,
					       "Solde_dernier_relevé" ))
				    SOLDE_DERNIER_RELEVE = my_strtod ( xmlNodeGetContent ( node_detail ),
								       NULL );

				if ( !strcmp ( node_detail -> name,
					       "Dernier_no_de_rapprochement" ))
				    DERNIER_NO_RAPPROCHEMENT = my_atoi ( xmlNodeGetContent ( node_detail ));

				if ( !strcmp ( node_detail -> name,
					       "Compte_cloturé" ))
				    COMPTE_CLOTURE = my_atoi ( xmlNodeGetContent ( node_detail ));

				if ( !strcmp ( node_detail -> name,
					       "Commentaires" ))
				    COMMENTAIRE = xmlNodeGetContent ( node_detail );

				if ( !strcmp ( node_detail -> name,
					       "Adresse_du_titulaire" ))
				    ADRESSE_TITULAIRE = xmlNodeGetContent ( node_detail );

				if ( !strcmp ( node_detail -> name,
					       "Type_défaut_débit" ))
				    TYPE_DEFAUT_DEBIT = my_atoi ( xmlNodeGetContent ( node_detail ));

				if ( !strcmp ( node_detail -> name,
					       "Type_défaut_crédit" ))
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
				node_detail = node_detail -> next;
			    }
			}

			/* dans certains cas d'import qif, le nom du compte peut être nul */
			/* dans ce cas le met à "" */

			if ( !NOM_DU_COMPTE )
			    NOM_DU_COMPTE = g_strdup ( "" );

			/* on récupère ici le détail des types */

			if ( !strcmp ( node_nom_comptes -> name,
				       "Détail_de_Types" ))
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
										    "Numérotation_auto" ));
				    type -> no_en_cours = my_atoi ( xmlGetProp ( node_type,
									      "No_en_cours" ));

				    type -> no_compte = NO_COMPTE;

				    TYPES_OPES = g_slist_append ( TYPES_OPES,
								  type );
				}

				node_type = node_type -> next;
			    }
			}


			/* on récupère ici le détail des opés */

			if ( !strcmp ( node_nom_comptes -> name,
				       "Détail_des_opérations" ))
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
				    operation -> no_operation = my_atoi ( xmlGetProp ( node_ope,
										    "No" ));

				    pointeur_char = g_strsplit ( xmlGetProp ( node_ope ,
									      "D" ),
								 "/",
								 3 );
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


				    /* on met le compte associé */

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

		    /*       la sélection au départ est en bas de la liste */

		    OPERATION_SELECTIONNEE = GINT_TO_POINTER ( -1 );


		    /* on incrémente p_tab_nom_de_compte_variable pour le compte suivant */

		    p_tab_nom_de_compte_variable++;
		}

		node_comptes = node_comptes -> next;
	    }
	}

	/* on récupère ici les échéances */

	if ( !strcmp ( node_1 -> name,
		       "Échéances" ))
	{
	    xmlNodePtr node_echeances;

	    /* node_echeances va faire le tour de l'arborescence des échéances */

	    node_echeances = node_1 -> children;

	    while ( node_echeances )
	    {
		/* on va récupérer ici les généralités des échéances */

		if ( !strcmp ( node_echeances -> name,
			       "Généralités" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_echeances -> children;

		    while ( node_generalites )
		    {
			if ( !strcmp ( node_generalites -> name,
				       "Nb_échéances" ))
			    nb_echeances = my_atoi ( xmlNodeGetContent ( node_generalites ));

			if ( !strcmp ( node_generalites -> name,
				       "No_dernière_échéance" ))
			    no_derniere_echeance = my_atoi ( xmlNodeGetContent ( node_generalites ));

			node_generalites = node_generalites -> next;
		    }
		}


		/* on va récupérer ici les échéances */

		if ( !strcmp ( node_echeances -> name,
			       "Détail_des_échéances" ))
		{
		    xmlNodePtr node_detail;

		    gsliste_echeances = NULL;

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
										  "Catégorie" ));

			    operation_echeance -> sous_categorie = my_atoi ( xmlGetProp ( node_detail,
										       "Sous-catégorie" ));

			    operation_echeance -> compte_virement = my_atoi ( xmlGetProp ( node_detail,
											"Virement_compte" ));

			    operation_echeance -> type_ope = my_atoi ( xmlGetProp ( node_detail,
										 "Type" ));

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
										    "Périodicité" ));

			    operation_echeance -> intervalle_periodicite_personnalisee = my_atoi ( xmlGetProp ( node_detail,
													     "Intervalle_périodicité" ));

			    operation_echeance -> periodicite_personnalisee = my_atoi ( xmlGetProp ( node_detail,
												  "Périodicité_personnalisée" ));

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


			    gsliste_echeances = g_slist_append ( gsliste_echeances,
								 operation_echeance);
			}

			node_detail = node_detail -> next;
		    }
		}
		node_echeances = node_echeances -> next;
	    }
	}

	/* on récupère ici les tiers */

	if ( !strcmp ( node_1 -> name,
		       "Tiers" ))
	{
	    xmlNodePtr node_tiers;

	    /* node_tiers va faire le tour de l'arborescence des tiers */

	    node_tiers = node_1 -> children;

	    while ( node_tiers )
	    {
		/* on va récupérer ici les généralités des tiers */

		if ( !strcmp ( node_tiers -> name,
			       "Généralités" ))
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


		/* on va récupérer ici les tiers */

		if ( !strcmp ( node_tiers -> name,
			       "Détail_des_tiers" ))
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
	}

	/* on récupère ici les catégories */

	if ( !strcmp ( node_1 -> name,
		       "Catégories" ))
	{
	    xmlNodePtr node_categories;

	    /* node_categories va faire le tour de l'arborescence des catégories */

	    node_categories = node_1 -> children;

	    while ( node_categories )
	    {
		/* on va récupérer ici les généralités des catégories */

		if ( !strcmp ( node_categories -> name,
			       "Généralités" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_categories -> children;

		    while ( node_generalites )
		    {
			if ( !strcmp ( node_generalites -> name,
				       "Nb_catégories" ))
			    nb_enregistrements_categories = my_atoi ( xmlNodeGetContent ( node_generalites ));

			if ( !strcmp ( node_generalites -> name,
				       "No_dernière_catégorie" ))
			    no_derniere_categorie = my_atoi ( xmlNodeGetContent ( node_generalites ));

			node_generalites = node_generalites -> next;
		    }
		}


		/* on va récupérer ici les catégories */

		if ( !strcmp ( node_categories -> name,
			       "Détail_des_catégories" ))
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
			    gint sous_categ_max = 0;
			    
			    categorie -> no_categ = my_atoi ( xmlGetProp ( node_detail,
									"No" ));
			    categorie -> nom_categ = xmlGetProp ( node_detail,
								  "Nom" );
			    categorie -> type_categ = my_atoi ( xmlGetProp ( node_detail,
									  "Type" ));
			    categorie -> no_derniere_sous_categ = my_atoi ( xmlGetProp ( node_detail,
										      "No_dernière_sous_cagégorie" ));

			    /*  pour chaque catégorie, on récupère les sous-catégories */

			    categorie -> liste_sous_categ = NULL;
			    node_sous_categ = node_detail -> children;

			    while ( node_sous_categ )
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
 
				/* Guard against false values due to a bug
				 * in previous versions of Grisbi. */
				if ( sous_categ -> no_sous_categ > sous_categ_max )
				{
				    sous_categ_max = sous_categ -> no_sous_categ;
				}
				
				node_sous_categ = node_sous_categ -> next;
			    }

			    /* Guard against false values due to a bug
			     * in previous versions of Grisbi. */
			    if ( categorie -> no_derniere_sous_categ != sous_categ_max )
			    {
				categorie -> no_derniere_sous_categ = sous_categ_max;
			    }

			    liste_struct_categories = g_slist_append ( liste_struct_categories,
								       categorie );
			}

			node_detail = node_detail -> next;
		    }
		}

		node_categories = node_categories -> next;
	    }
	    /* création de la liste des catég pour le combofix */

	    creation_liste_categ_combofix ();
	}

	/* on récupère ici les imputations */

	if ( !strcmp ( node_1 -> name,
		       "Imputations" ))
	{
	    xmlNodePtr node_imputations;

	    /* node_imputations va faire le tour de l'arborescence des imputations */

	    node_imputations = node_1 -> children;

	    while ( node_imputations )
	    {
		/* on va récupérer ici les généralités des imputations */

		if ( !strcmp ( node_imputations -> name,
			       "Généralités" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_imputations -> children;

		    while ( node_generalites )
		    {
			if ( !strcmp ( node_generalites -> name,
				       "Nb_imputations" ))
			    nb_enregistrements_imputations = my_atoi ( xmlNodeGetContent ( node_generalites ));

			if ( !strcmp ( node_generalites -> name,
				       "No_dernière_imputation" ))
			    no_derniere_imputation = my_atoi ( xmlNodeGetContent ( node_generalites ));

			node_generalites = node_generalites -> next;
		    }
		}


		/* on va récupérer ici les imputations */

		if ( !strcmp ( node_imputations -> name,
			       "Détail_des_imputations" ))
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
											    "No_dernière_sous_imputation" ));

			    /*  pour chaque catégorie, on récupère les sous-catégories */

			    imputation -> liste_sous_imputation = NULL;
			    node_sous_imputation = node_detail -> children;

			    while ( node_sous_imputation )
			    {
				struct struct_sous_imputation *sous_imputation;

				sous_imputation = calloc ( 1,
							   sizeof ( struct struct_sous_imputation ) );

				sous_imputation -> no_sous_imputation = my_atoi ( latin2utf8(xmlGetProp ( node_sous_imputation,
												       "No" )));
				sous_imputation -> nom_sous_imputation = latin2utf8(xmlGetProp ( node_sous_imputation,
												 "Nom" ));

				imputation -> liste_sous_imputation = g_slist_append ( imputation -> liste_sous_imputation,
										       sous_imputation );
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
	    /* création de la liste des imputations pour le combofix */

	    creation_liste_imputation_combofix ();
	}


	/* on récupère ici les devises */

	if ( !strcmp ( node_1 -> name,
		       "Devises" ))
	{
	    xmlNodePtr node_devises;

	    /* node_devises va faire le tour de l'arborescence des devises */

	    node_devises = node_1 -> children;

	    while ( node_devises )
	    {
		/* on va récupérer ici les généralités des devises */

		if ( !strcmp ( node_devises -> name,
			       "Généralités" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_devises -> children;

		    while ( node_generalites )
		    {
			if ( !strcmp ( node_generalites -> name,
				       "Nb_devises" ))
			    nb_devises = my_atoi ( latin2utf8(xmlNodeGetContent ( node_generalites )));

			if ( !strcmp ( node_generalites -> name,
				       "No_dernière_devise" ))
			    no_derniere_devise = my_atoi ( latin2utf8(xmlNodeGetContent ( node_generalites )));

			node_generalites = node_generalites -> next;
		    }
		}


		/* on va récupérer ici les devises */

		if ( !strcmp ( node_devises -> name,
			       "Détail_des_devises" ))
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
			    devise -> no_devise = my_atoi ( latin2utf8(xmlGetProp ( node_detail,
										 "No" )));
			    devise -> nom_devise = latin2utf8(xmlGetProp ( node_detail,
									   "Nom" ));
			    devise -> code_devise = latin2utf8(xmlGetProp ( node_detail,
									    "Code" ));
			    if ( !strlen ( devise -> code_devise ))
				devise -> code_devise = NULL;
			    /* Handle Euro nicely */
			    if (! strcmp (devise -> nom_devise, "Euro"))
			    {
				devise -> code_devise = "â¬";
				devise -> code_iso4217_devise = g_strdup ("EUR");
			    }

			    devise -> passage_euro = my_atoi ( latin2utf8(xmlGetProp ( node_detail,
										    "Passage_euro" )));

			    if ( strlen ( latin2utf8(xmlGetProp ( node_detail,
								  "Date_dernier_change" ))))
			    {
				gchar **pointeur_char;

				pointeur_char = g_strsplit ( latin2utf8(xmlGetProp ( node_detail,
										     "Date_dernier_change" )),
							     "/",
							     3 );

				devise -> date_dernier_change = g_date_new_dmy ( my_atoi ( pointeur_char[0] ),
										 my_atoi ( pointeur_char[1] ),
										 my_atoi ( pointeur_char[2] ));
				g_strfreev ( pointeur_char );
			    }
			    else
				devise -> date_dernier_change = NULL;

			    devise -> une_devise_1_egale_x_devise_2 = my_atoi ( latin2utf8(xmlGetProp ( node_detail,
												     "Rapport_entre_devises" )));
			    devise -> no_devise_en_rapport = my_atoi ( latin2utf8(xmlGetProp ( node_detail,
											    "Devise_en_rapport" )));
			    devise -> change = my_strtod ( latin2utf8(xmlGetProp ( node_detail,
										   "Change" )),
							   NULL );

			    liste_struct_devises = g_slist_append ( liste_struct_devises,
								    devise );
			}

			node_detail = node_detail -> next;
		    }
		}
		node_devises = node_devises -> next;
	    }
	}

	/* on récupère ici les banques */

	if ( !strcmp ( node_1 -> name,
		       "Banques" ))
	{
	    xmlNodePtr node_banques;

	    /* node_banques va faire le tour de l'arborescence des banques */

	    node_banques = node_1 -> children;

	    while ( node_banques )
	    {
		/* on va récupérer ici les généralités des banques */

		if ( !strcmp ( node_banques -> name,
			       "Généralités" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_banques -> children;

		    while ( node_generalites )
		    {
			if ( !strcmp ( node_generalites -> name,
				       "Nb_banques" ))
			    nb_banques = my_atoi ( latin2utf8(xmlNodeGetContent ( node_generalites )));

			if ( !strcmp ( node_generalites -> name,
				       "No_dernière_banque" ))
			    no_derniere_banque = my_atoi ( latin2utf8(xmlNodeGetContent ( node_generalites )));


			node_generalites = node_generalites -> next;
		    }
		}


		/* on va récupérer ici les banques */

		if ( !strcmp ( node_banques -> name,
			       "Détail_des_banques" ))
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
			    banque -> no_banque = my_atoi ( latin2utf8(xmlGetProp ( node_detail,
										 "No" )));
			    banque -> nom_banque = latin2utf8(xmlGetProp ( node_detail,
									   "Nom" ));
			    banque -> code_banque = latin2utf8(xmlGetProp ( node_detail,
									    "Code" ));
			    if ( !strlen ( banque -> code_banque ))
				banque -> code_banque = NULL;
			    banque -> adr_banque = latin2utf8(xmlGetProp ( node_detail,
									   "Adresse" ));
			    if ( !strlen ( banque -> adr_banque ))
				banque -> adr_banque = NULL;
			    banque -> tel_banque = latin2utf8(xmlGetProp ( node_detail,
									   "Tél" ));
			    if ( !strlen ( banque -> tel_banque ))
				banque -> tel_banque = NULL;
			    banque -> email_banque = latin2utf8(xmlGetProp ( node_detail,
									     "Mail" ));
			    if ( !strlen ( banque -> email_banque ))
				banque -> email_banque = NULL;
			    banque -> web_banque = latin2utf8(xmlGetProp ( node_detail,
									   "Web" ));
			    if ( !strlen ( banque -> web_banque ))
				banque -> web_banque = NULL;
			    banque -> nom_correspondant = latin2utf8(xmlGetProp ( node_detail,
										  "Nom_correspondant" ));
			    if ( !strlen ( banque -> nom_correspondant ))
				banque -> nom_correspondant = NULL;
			    banque -> fax_correspondant = latin2utf8(xmlGetProp ( node_detail,
										  "Fax_correspondant" ));
			    if ( !strlen ( banque -> fax_correspondant ))
				banque -> fax_correspondant = NULL;
			    banque -> tel_correspondant = latin2utf8(xmlGetProp ( node_detail,
										  "Tél_correspondant" ));
			    if ( !strlen ( banque -> tel_correspondant ))
				banque -> tel_correspondant = NULL;
			    banque -> email_correspondant = latin2utf8(xmlGetProp ( node_detail,
										    "Mail_correspondant" ));
			    if ( !strlen ( banque -> email_correspondant ))
				banque -> email_correspondant = NULL;
			    banque -> remarque_banque = latin2utf8(xmlGetProp ( node_detail,
										"Remarques" ));
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
	}

	/* on récupère ici les exercices */

	if ( !strcmp ( node_1 -> name,
		       "Exercices" ))
	{
	    xmlNodePtr node_exercices;

	    /* node_exercices va faire le tour de l'arborescence des exercices */

	    node_exercices = node_1 -> children;

	    while ( node_exercices )
	    {
		/* on va récupérer ici les généralités des exercices */

		if ( !strcmp ( node_exercices -> name,
			       "Généralités" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_exercices -> children;

		    while ( node_generalites )
		    {
			if ( !strcmp ( node_generalites -> name,
				       "Nb_exercices" ))
			    nb_exercices = my_atoi ( latin2utf8(xmlNodeGetContent ( node_generalites )));

			if ( !strcmp ( node_generalites -> name,
				       "No_dernier_exercice" ))
			    no_derniere_exercice = my_atoi ( latin2utf8(xmlNodeGetContent ( node_generalites )));

			node_generalites = node_generalites -> next;
		    }
		}


		/* on va récupérer ici les exercices */

		if ( !strcmp ( node_exercices -> name,
			       "Détail_des_exercices" ))
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
			    exercice -> no_exercice = my_atoi ( latin2utf8(xmlGetProp ( node_detail,
										     "No" )));
			    exercice -> nom_exercice = latin2utf8(xmlGetProp ( node_detail,
									       "Nom" ));

			    if ( strlen ( latin2utf8(xmlGetProp ( node_detail,
								  "Date_début" ))))
			    {
				gchar **pointeur_char;

				pointeur_char = g_strsplit ( latin2utf8(xmlGetProp ( node_detail,
										     "Date_début" )),
							     "/",
							     3 );

				exercice -> date_debut = g_date_new_dmy ( my_atoi ( pointeur_char[0] ),
									  my_atoi ( pointeur_char[1] ),
									  my_atoi ( pointeur_char[2] ));
				g_strfreev ( pointeur_char );
			    }
			    else
				exercice -> date_debut = NULL;

			    if ( strlen ( latin2utf8(xmlGetProp ( node_detail,
								  "Date_fin" ))))
			    {
				gchar **pointeur_char;

				pointeur_char = g_strsplit ( latin2utf8(xmlGetProp ( node_detail,
										     "Date_fin" )),
							     "/",
							     3 );

				exercice -> date_fin = g_date_new_dmy ( my_atoi ( pointeur_char[0] ),
									my_atoi ( pointeur_char[1] ),
									my_atoi ( pointeur_char[2] ));
				g_strfreev ( pointeur_char );
			    }
			    else
				exercice -> date_fin = NULL;

			    exercice -> affiche_dans_formulaire = my_atoi ( latin2utf8(xmlGetProp ( node_detail,
												 "Affiché" )));

			    liste_struct_exercices = g_slist_append ( liste_struct_exercices,
								      exercice );
			}

			node_detail = node_detail -> next;
		    }
		}
		node_exercices = node_exercices -> next;
	    }
	}

	/* on récupère ici les rapprochements */

	if ( !strcmp ( node_1 -> name,
		       "Rapprochements" ))
	{
	    xmlNodePtr node_rapprochements;

	    /* node_rapprochements va faire le tour de l'arborescence des rapprochements */

	    node_rapprochements = node_1 -> children;

	    while ( node_rapprochements )
	    {
		/* il n'y a pas de généralités ... */

		/* on va récupérer ici les rapprochements */

		if ( !strcmp ( node_rapprochements -> name,
			       "Détail_des_rapprochements" ))
		{
		    xmlNodePtr node_detail;

		    liste_no_rapprochements = NULL;
		    node_detail = node_rapprochements -> children;

		    while ( node_detail )
		    {
			struct struct_no_rapprochement *rapprochement;

			rapprochement = calloc ( 1,
						 sizeof ( struct struct_no_rapprochement ));

			if ( node_detail -> type != XML_TEXT_NODE )
			{
			    rapprochement -> no_rapprochement = my_atoi ( latin2utf8(xmlGetProp ( node_detail,
											       "No" )));
			    rapprochement -> nom_rapprochement = latin2utf8(xmlGetProp ( node_detail,
											 "Nom" ));

			    rapprochement -> nom_rapprochement = g_strstrip ( rapprochement -> nom_rapprochement);

			    liste_no_rapprochements = g_slist_append ( liste_no_rapprochements,
								       rapprochement );
			}

			node_detail = node_detail -> next;
		    }
		}
		node_rapprochements = node_rapprochements -> next;
	    }
	}
	node_1 = node_1 -> next;
    }


    /* on libère la mémoire */

    xmlFreeDoc ( doc );

    supprime_operations_orphelines ();


    /*   on applique la modif des ventils : si une opé ventilée est < 0, les opés de ventil ont le même signe */
    /* que l'opé ventilée */
    /*   pour ça, on fait le tour de toutes les opés, et si on a une opé de ventil, on vérifie le signe de la ventil */
    /*     associée, si elle est négative, on inverse le signe */

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	GSList *pointeur_tmp;
	struct structure_operation *operation_associee;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
	operation_associee = NULL;

	pointeur_tmp = LISTE_OPERATIONS;

	while ( pointeur_tmp )
	{
	    struct structure_operation *operation;

	    operation = pointeur_tmp -> data;

	    if ( operation -> montant
		 &&
		 operation -> no_operation_ventilee_associee )
	    {
		if ( !operation_associee
		     ||
		     operation -> no_operation_ventilee_associee != operation_associee -> no_operation )
		{
		    GSList *pointeur_2;

		    pointeur_2 = g_slist_find_custom ( LISTE_OPERATIONS,
						       GINT_TO_POINTER ( operation -> no_operation_ventilee_associee ),
						       (GCompareFunc) recherche_operation_par_no );

		    if ( pointeur_2 )
			operation_associee = pointeur_2 -> data;
		}

		if ( operation_associee
		     &&
		     operation_associee -> montant < 0 )
		    operation -> montant = -operation -> montant;
	    }
	    pointeur_tmp = pointeur_tmp -> next;
	}
    }

    /* met l'affichage des opés comme il l'était avant */

    initialise_tab_affichage_ope();

    /*   la taille des colonnes est automatique, on y met les anciens rapports */

    etat.largeur_auto_colonnes = 1;
    rapport_largeur_colonnes[0] = 11;
    rapport_largeur_colonnes[1] = 13;
    rapport_largeur_colonnes[2] = 30;
    rapport_largeur_colonnes[3] = 3;
    rapport_largeur_colonnes[4] = 11;
    rapport_largeur_colonnes[5] = 11;
    rapport_largeur_colonnes[6] = 11;


    /* on marque le fichier comme ouvert */

    fichier_marque_ouvert ( TRUE );

    modification_fichier ( TRUE );

    return ( TRUE );
}
/***********************************************************************************************************/

/***********************************************************************************************************/
/* un bug dans les versions antérieurs à la 0.4 pouvait créer des opés de ventil sans ventilation associée */
/* ou des virements sans contre opération */
/* on retrouve ces opés, supprime les opés de ventil orphelines et met les relations de virement orphelin */
/* à 0 */
/***********************************************************************************************************/

void supprime_operations_orphelines ( void )
{
    gint i;
    gint nb_vir;
    gint nb_ventil;

    nb_vir = 0;
    nb_ventil = 0;

    for ( i = 0 ; i < nb_comptes ; i++ )
    {
	GSList *liste_tmp;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	liste_tmp = LISTE_OPERATIONS;

	while ( liste_tmp )
	{
	    struct structure_operation *operation;
	    GSList *result;

	    operation = liste_tmp -> data;

	    /* si c'est un virement, recherche la contre opération */

	    /* 	  on vérifie que relation_no_compte > 0 (des fois négatif, bug qui vient de je sais pas où) */

	    if ( operation -> relation_no_operation )
	    {
		/* 	      si relation_no_compte = -1, c'est un virement vers compte suprimé, donc on s'en fout */
		/* 		si relation_no_compte < -1, c'est un bug corrigé, on met à -1 */

		if ( operation -> relation_no_compte >= 0 )
		{
		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

		    result = g_slist_find_custom ( LISTE_OPERATIONS,
						   GINT_TO_POINTER ( operation -> relation_no_operation ),
						   (GCompareFunc) recherche_operation_par_no );

		    /* si la contre opération n'est pas trouvée, on met les relations de cette opé à 0 */

		    if ( !result )
		    {
			nb_vir++;
			operation -> relation_no_operation = 0;
			operation -> relation_no_compte = 0;
		    }

		    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		}
		else
		{
		    if ( operation -> relation_no_compte < -1 )
			operation -> relation_no_compte = -1;
		}
	    }

	    /* si c'est une opé de ventil, recherche la ventil associée */

	    if ( operation -> no_operation_ventilee_associee )
	    {
		result = g_slist_find_custom ( LISTE_OPERATIONS,
					       GINT_TO_POINTER ( operation -> no_operation_ventilee_associee ),
					       (GCompareFunc) recherche_operation_par_no );

		/*  si on a trouvé une opé, vérifie que c'est une ventil */
		/* sinon, supprime l'opé de ventil */

		if ( result )
		{
		    struct structure_operation *ope_ventil;

		    ope_ventil = result -> data;

		    if ( ope_ventil -> operation_ventilee )
			liste_tmp = liste_tmp -> next;
		    else
		    {
			/* supprime l'opération dans la liste des opés */

			liste_tmp = liste_tmp -> next;
			LISTE_OPERATIONS = g_slist_remove ( LISTE_OPERATIONS,
							    operation );
			free ( operation );
			NB_OPE_COMPTE--;
			nb_ventil++;
		    }

		}
		else
		{
		    /* supprime l'opération dans la liste des opés */

		    liste_tmp = liste_tmp -> next;
		    LISTE_OPERATIONS = g_slist_remove ( LISTE_OPERATIONS,
							operation );
		    free ( operation );
		    NB_OPE_COMPTE--;
		    nb_ventil++;
		}
	    }
	    else
		liste_tmp = liste_tmp -> next;
	}
    }

    if ( nb_ventil
	 ||
	 nb_vir )
    {
	gchar *message;

	/* FIXME */
	message = _("There was a bug in versions before 0.4.0, which could lead to orphan transactions (either transfers without countra-transaction, or breakdown transactions details without associated breakdown transaction.\n\nGrisbi searched for such transaction and found:\n\n");

	if ( nb_ventil )
	    message = g_strconcat ( message,
				    utils_itoa ( nb_ventil ),
				    " ",
				    _("breakdown transactions, all of them were deleted.\n"),
				    NULL );

	if ( nb_vir )
	    message = g_strconcat ( message,
				    utils_itoa ( nb_vir ),
				    " ",
				    _("transfers without contra-transaction, which categories were deleted.\n"),
				    NULL );

	message = g_strconcat ( message,
				"\n",
				_("These modifications should not impact on neither Grisbi's behavior nor your accounts balances."),
				NULL );



	dialogue ( message );
    }
    /*     ajout de la 0.5 -> valeur_echelle_recherche_date_import qu'on me à 2 */

    valeur_echelle_recherche_date_import = 2;

    etat.en_train_de_charger = 0;
}
/***********************************************************************************************************/


/*****************************************************************************/
/* version 0.4.0 */
/*****************************************************************************/

gboolean charge_operations_version_0_4_0 ( xmlDocPtr doc )
{
    gint retour;
    gint i;

    /* il n'y a aucune différence de struct entre la 0.4.0 et la 0.4.1 */
    /* sauf que la 0.4.0 n'attribuait pas le no de relevé aux opés filles */
    /* d'une ventilation */

    retour = charge_operations_version_0_4_1 (doc);

    if ( !retour )
	return ( FALSE );

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

    return ( TRUE );
}
/*****************************************************************************/



/*****************************************************************************/
/* version 0.4.1 */
/*****************************************************************************/
gboolean charge_operations_version_0_4_1 ( xmlDocPtr doc )
{
    xmlNodePtr node_1;
    xmlNodePtr root = xmlDocGetRootElement(doc);
    struct stat buffer_stat;


    etat.en_train_de_charger = 1;

    /* on place node_1 sur les generalites */

    node_1 = root -> children;
    if ( node_1 )
	node_1 = node_1 -> next;

    /*   on met en place la boucle de node_1, qui va successivement passer */
    /*     par les generalites, les comptes, les echeances ... */

    while ( node_1 )
    {

	/* on recupère ici les generalites */

	if ( !strcmp ( node_1 -> name,
		       "Generalites" ) )
	{
	    xmlNodePtr node_generalites;

	    /* node_generalites va faire le tour des generalites */

	    node_generalites = node_1 -> children;
	    if ( node_generalites )
		node_generalites = node_generalites -> next;

	    while ( node_generalites )
	    {
		if ( !strcmp ( node_generalites -> name,
			       "Fichier_ouvert" ))
		    if ( (etat.fichier_deja_ouvert  = my_atoi ( xmlNodeGetContent ( node_generalites ))))
		    {
			dialogue_conditional_hint ( g_strdup_printf( _("File \"%s\" is already opened"), nom_fichier_comptes),
						    _("Either this file is already opened by another user or it wasn't closed correctly (maybe Grisbi crashed?).\nGrisbi can't save the file unless you activate the \"Force saving locked files\" option in setup."), 
						    &(etat.display_message_lock_active) );
		    }


		if ( !strcmp ( node_generalites -> name,
			       "Backup" ))
		    nom_fichier_backup = xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Titre" ))
		    titre_fichier = xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Adresse_commune" ))
		    adresse_commune	= xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Adresse_secondaire" ))
		    adresse_secondaire = xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_exercices" ))
		    etat.utilise_exercice = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_IB" ))
		    etat.utilise_imputation_budgetaire = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_PC" ))
		    etat.utilise_piece_comptable = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_info_BG" ))
		    etat.utilise_info_banque_guichet = my_atoi ( xmlNodeGetContent ( node_generalites ));

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
			       "Chemin_logo" ))
		    chemin_logo = xmlNodeGetContent ( node_generalites );

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





		node_generalites = node_generalites -> next;
	    }
	}

/* 	s'il y avait un ancien logo mais qu'il n'existe plus, on met le logo par défaut */

	if ( !chemin_logo
	     ||
	     !strlen ( chemin_logo )
	     ||
	     ( chemin_logo
	       &&
	       strlen ( chemin_logo )
	       &&
	       utf8_stat ( chemin_logo, &buffer_stat) == -1 ))
	    chemin_logo = g_strdup ( LOGO_PATH );


	/* on recupère ici les comptes et operations */

	if ( !strcmp ( node_1 -> name,
		       "Comptes" ))
	{
	    xmlNodePtr node_comptes;

	    /* node_comptes va faire le tour de l'arborescence des comptes */

	    node_comptes = node_1 -> children;
	    if ( node_comptes )
		node_comptes = node_comptes -> next;

	    while ( node_comptes )
	    {

		/* on va recuperer ici les generalites des comptes */

		if ( !strcmp ( node_comptes -> name,
			       "Generalites" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_comptes -> children;
		    if ( node_generalites )
			node_generalites = node_generalites -> next;

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
			    p_tab_nom_de_compte_courant = p_tab_nom_de_compte + compte_courant;
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
		    if ( node_nom_comptes )
			node_nom_comptes = node_nom_comptes -> next;

		    while ( node_nom_comptes )
		    {
			/* on recupère les details du compte */

			if ( !strcmp ( node_nom_comptes -> name,
				       "Details" ))
			{
			    xmlNodePtr node_detail;

			    node_detail = node_nom_comptes -> children;
			    if ( node_detail )
				node_detail = node_detail -> next;

			    while ( node_detail )
			    {

				if ( !strcmp ( node_detail -> name,
					       "Nom" ))
				    NOM_DU_COMPTE = xmlNodeGetContent ( node_detail );

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
					       "Nb_operations" ))
				    NB_OPE_COMPTE = my_atoi ( xmlNodeGetContent ( node_detail ));

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
					       "Solde_courant" ))
				    SOLDE_COURANT = my_strtod ( xmlNodeGetContent ( node_detail ),
								NULL );

				if ( !strcmp ( node_detail -> name,
					       "Date_dernier_releve" ))
				{
				    gchar **pointeur_char;

				    if ( xmlNodeGetContent (node_detail) &&
					 strlen (xmlNodeGetContent (node_detail)) > 0 )
				    {
					pointeur_char = g_strsplit ( xmlNodeGetContent ( node_detail ),
								     "/", 3 );
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

				if ( node_type -> type != XML_TEXT_NODE )
				{
				    type = calloc ( 1, sizeof ( struct struct_type_ope ));

				    type -> no_type = my_atoi ( xmlGetProp ( node_type, "No" ));
				    type -> nom_type = xmlGetProp ( node_type, "Nom" );
				    type -> signe_type = my_atoi ( xmlGetProp ( node_type,
									     "Signe" ));
				    type -> affiche_entree = my_atoi ( xmlGetProp ( node_type,
										 "Affiche_entree" ));
				    type -> numerotation_auto = my_atoi ( xmlGetProp ( node_type,
										    "Numerotation_auto" ));
				    type -> no_en_cours = my_atoi ( xmlGetProp ( node_type,
									      "No_en_cours" ));
				    type -> no_compte = NO_COMPTE;
				    TYPES_OPES = g_slist_append ( TYPES_OPES, type );
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

			    if (node_ope)
				node_ope = node_ope -> next;

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

				    pointeur_char = g_strsplit ( xmlGetProp ( node_ope , "D" ), "/", 3 );
				    operation -> jour = my_atoi ( pointeur_char[0] );
				    operation -> mois = my_atoi ( pointeur_char[1] );
				    operation -> annee = my_atoi ( pointeur_char[2] );
				    operation -> date = g_date_new_dmy ( operation -> jour,
									 operation -> mois,
									 operation -> annee );
				    g_strfreev ( pointeur_char );

				    /* GDC prise en compte de la lecture de la date bancaire */

				    pointeur = xmlGetProp ( node_ope, "Db" );

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

				    operation -> montant = my_strtod ( xmlGetProp ( node_ope, "M" ), NULL );
				    operation -> devise = my_atoi ( xmlGetProp ( node_ope, "De" ));
				    operation -> une_devise_compte_egale_x_devise_ope = my_atoi ( xmlGetProp ( node_ope, "Rdc" ));
				    operation -> taux_change = my_strtod ( xmlGetProp ( node_ope, "Tc" ), NULL );

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

		    OPERATION_SELECTIONNEE = GINT_TO_POINTER ( -1 );


		    /* on incremente p_tab_nom_de_compte_variable pour le compte suivant */

		    p_tab_nom_de_compte_variable++;
		}

		node_comptes = node_comptes -> next;
	    }
	}

	/* on recupère ici les echeances */

	if ( !strcmp ( node_1 -> name,
		       "Echeances" ))
	{
	    xmlNodePtr node_echeances;

	    /* node_echeances va faire le tour de l'arborescence des echeances */

	    node_echeances = node_1 -> children;
	    if ( node_echeances )
		node_echeances = node_echeances -> next;

	    while ( node_echeances )
	    {
		/* on va recuperer ici les generalites des echeances */

		if ( !strcmp ( node_echeances -> name,
			       "Generalites" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_echeances -> children;
		    if ( node_generalites )
			node_generalites = node_generalites -> next;

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

		    gsliste_echeances = NULL;

		    node_detail = node_echeances -> children;
		    if ( node_detail )
			node_detail = node_detail -> next;

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


			    gsliste_echeances = g_slist_append ( gsliste_echeances,
								 operation_echeance);

			}

			node_detail = node_detail -> next;
		    }
		}
		node_echeances = node_echeances -> next;
	    }
	}

	/* on recupère ici les tiers */

	if ( !strcmp ( node_1 -> name,
		       "Tiers" ))
	{
	    xmlNodePtr node_tiers;

	    /* node_tiers va faire le tour de l'arborescence des tiers */

	    node_tiers = node_1 -> children;
	    if ( node_tiers )
		node_tiers = node_tiers -> next;

	    while ( node_tiers )
	    {
		/* on va recuperer ici les generalites des tiers */

		if ( !strcmp ( node_tiers -> name,
			       "Generalites" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_tiers -> children;
		    if ( node_generalites )
			node_generalites = node_generalites -> next;

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
		    if ( node_detail )
			node_detail = node_detail -> next;

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
	}

	/* on recupère ici les categories */

	if ( !strcmp ( node_1 -> name,
		       "Categories" ))
	{
	    xmlNodePtr node_categories;

	    /* node_categories va faire le tour de l'arborescence des categories */

	    node_categories = node_1 -> children;
	    if ( node_categories )
		node_categories = node_categories -> next;

	    while ( node_categories )
	    {
		/* on va recuperer ici les generalites des categories */

		if ( !strcmp ( node_categories -> name,
			       "Generalites" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_categories -> children;
		    if ( node_generalites )
			node_generalites = node_generalites -> next;

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
		    if ( node_detail )
			node_detail = node_detail -> next;

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
			node_detail = node_detail -> next;
		    }
		}
		node_categories = node_categories -> next;
	    }
	    /* creation de la liste des categ pour le combofix */

	    creation_liste_categ_combofix ();
	}

	/* on recupère ici les imputations */

	if ( !strcmp ( node_1 -> name,
		       "Imputations" ))
	{
	    xmlNodePtr node_imputations;

	    /* node_imputations va faire le tour de l'arborescence des imputations */

	    node_imputations = node_1 -> children;
	    if ( node_imputations )
		node_imputations = node_imputations -> next;

	    while ( node_imputations )
	    {
		/* on va recuperer ici les generalites des imputations */

		if ( !strcmp ( node_imputations -> name,
			       "Generalites" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_imputations -> children;
		    if ( node_generalites )
			node_generalites = node_generalites -> next;

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
			    gint sous_ib_max = 0;

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
				    /* Guard against false values due to a bug
				     * in previous versions of Grisbi. */
				    if ( sous_imputation -> no_sous_imputation > sous_ib_max )
					sous_ib_max = sous_imputation -> no_sous_imputation;

				}
 			    
				node_sous_imputation = node_sous_imputation -> next;
			    }

			    /* Guard against false values due to a bug
			     * in previous versions of Grisbi. */
			    if ( imputation -> no_derniere_sous_imputation != sous_ib_max )
				imputation -> no_derniere_sous_imputation = sous_ib_max;

			    liste_struct_imputation = g_slist_append ( liste_struct_imputation,
								       imputation );
			}

			node_detail = node_detail -> next;
		    }
		}
		node_imputations = node_imputations -> next;
	    }
	    /* creation de la liste des imputations pour le combofix */

	    creation_liste_imputation_combofix ();
	}


	/* on recupère ici les devises */

	if ( !strcmp ( node_1 -> name,
		       "Devises" ))
	{
	    xmlNodePtr node_devises;

	    /* node_devises va faire le tour de l'arborescence des devises */

	    node_devises = node_1 -> children;
	    if ( node_devises )
		node_devises = node_devises -> next;

	    while ( node_devises )
	    {
		/* on va recuperer ici les generalites des devises */

		if ( !strcmp ( node_devises -> name,
			       "Generalites" ))
		{
		    xmlNodePtr node_generalites;

		    node_generalites = node_devises -> children;
		    if ( node_generalites )
			node_generalites = node_generalites -> next;

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
		    if ( node_detail )
			node_detail = node_detail -> next;

		    while ( node_detail )
		    {
			struct struct_devise *devise;

			devise = calloc ( 1, sizeof ( struct struct_devise ));

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
			    /* Handle Euro nicely */
			    if (! strcmp (devise -> nom_devise, "Euro"))
			    {
				devise -> code_devise = "â¬";
				devise -> code_iso4217_devise = g_strdup ("EUR");
			    }

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
	}

	/* on recupère ici les banques */

	if ( !strcmp ( node_1 -> name,
		       "Banques" ))
	{
	    xmlNodePtr node_banques;

	    /* node_banques va faire le tour de l'arborescence des banques */

	    node_banques = node_1 -> children;
	    if ( node_banques )
		node_banques = node_banques -> next;

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
	}

	/* on recupère ici les exercices */

	if ( !strcmp ( node_1 -> name,
		       "Exercices" ))
	{
	    xmlNodePtr node_exercices;

	    /* node_exercices va faire le tour de l'arborescence des exercices */

	    node_exercices = node_1 -> children;

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

			exercice = calloc ( 1, sizeof ( struct struct_exercice ));

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
	}

	/* on recupère ici les rapprochements */

	if ( !strcmp ( node_1 -> name,
		       "Rapprochements" ))
	{
	    xmlNodePtr node_rapprochements;

	    /* node_rapprochements va faire le tour de l'arborescence des rapprochements */

	    node_rapprochements = node_1 -> children;

	    while ( node_rapprochements )
	    {
		/* il n'y a pas de generalites ... */

		/* on va recuperer ici les rapprochements */

		if ( !strcmp ( node_rapprochements -> name,
			       "Detail_des_rapprochements" ))
		{
		    xmlNodePtr node_detail;

		    liste_no_rapprochements = NULL;
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

			    liste_no_rapprochements = g_slist_append ( liste_no_rapprochements,
								       rapprochement );
			}

			node_detail = node_detail -> next;
		    }
		}
		node_rapprochements = node_rapprochements -> next;
	    }
	}

	/* on recupère ici les etats */

	if ( !strcmp ( node_1 -> name, "Etats" ))
	{
	    xmlNodePtr node_etats;

	    /* node_etats va faire le tour de l'arborescence des etats */

	    node_etats = node_1 -> children;

	    while ( node_etats )
	    {
		/* on va recuperer ici les generalites des etats */

		if ( node_etats -> type != XML_TEXT_NODE )
		{
		    if ( !strcmp ( node_etats -> name, "Generalites" ))
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

			    if ( node_detail -> type != XML_TEXT_NODE &&
				 !strcmp ( node_detail -> name, "Etat") )
			    {

				/* création du nouvel état */

				etat = calloc ( 1,
						sizeof ( struct struct_etat ));
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
						       "Aff_verif_ope" ))
					    etat -> afficher_verif_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

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

					if ( !strcmp (node_detail_etat -> name, "Date_debut") &&
					     xmlNodeGetContent (node_detail_etat) &&
					     strlen(xmlNodeGetContent (node_detail_etat)) > 0 )
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
					     xmlNodeGetContent(node_detail_etat) &&
					     strlen(xmlNodeGetContent(node_detail_etat)) > 0 )
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

				liste_struct_etats = g_slist_append ( liste_struct_etats,
								      etat );
			    }
			    node_detail = node_detail -> next;
			}
		    }
		}
		node_etats = node_etats -> next;
	    }
	}
	node_1 = node_1 -> next;
    }

    /*     ajout de la 0.5 -> valeur_echelle_recherche_date_import qu'on me à 2 */

    valeur_echelle_recherche_date_import = 2;

    /* on libère la memoire */

    xmlFreeDoc ( doc );

    etat.en_train_de_charger = 0;

    /* on marque le fichier comme ouvert */

    fichier_marque_ouvert ( TRUE );

    modification_fichier ( TRUE );

    return ( TRUE );
}
/***********************************************************************************************************/




/***********************************************************************************************************/
gboolean charge_operations_version_0_5_0 ( xmlDocPtr doc )
{
    xmlNodePtr node_1;
    xmlNodePtr root = xmlDocGetRootElement(doc);
    struct stat buffer_stat;


    etat.en_train_de_charger = 1;

    /* on place node_1 sur les generalites */

    node_1 = root -> children;

    /*   on met en place la boucle de node_1, qui va successivement passer */
    /*     par les generalites, les comptes, les echeances ... */

    while ( node_1 )
    {

	/* on recupère ici les generalites */

	if ( !strcmp ( node_1 -> name,
		       "Generalites" ) )
	{
	    xmlNodePtr node_generalites;

	    /* node_generalites va faire le tour des generalites */

	    node_generalites = node_1 -> children;

	    while ( node_generalites )
	    {
		if ( !strcmp ( node_generalites -> name,
			       "Fichier_ouvert" ))
		    if ( (etat.fichier_deja_ouvert  = my_atoi ( xmlNodeGetContent ( node_generalites ))))
		    {
			dialogue_conditional_hint ( g_strdup_printf( _("File \"%s\" is already opened"), nom_fichier_comptes),
						    _("Either this file is already opened by another user or it wasn't closed correctly (maybe Grisbi crashed?).\nGrisbi can't save the file unless you activate the \"Force saving locked files\" option in setup."), 
						    &(etat.display_message_lock_active) );
		    }


		if ( !strcmp ( node_generalites -> name,
			       "Backup" ))
		    nom_fichier_backup = xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Titre" ))
		    titre_fichier = xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Adresse_commune" ))
		    adresse_commune	= xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Adresse_secondaire" ))
		    adresse_secondaire = xmlNodeGetContent ( node_generalites );

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_exercices" ))
		    etat.utilise_exercice = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_IB" ))
		    etat.utilise_imputation_budgetaire = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_PC" ))
		    etat.utilise_piece_comptable = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Utilise_info_BG" ))
		    etat.utilise_info_banque_guichet = my_atoi ( xmlNodeGetContent ( node_generalites ));

		if ( !strcmp ( node_generalites -> name,
			       "Numero_devise_totaux_tiers" ))
		    no_devise_totaux_tiers = my_atoi ( xmlNodeGetContent ( node_generalites ));
		if ( !strcmp ( node_generalites -> name,
			       "Numero_devise_totaux_ib" ))
		    no_devise_totaux_ib = my_atoi ( xmlNodeGetContent ( node_generalites ));
		if ( !strcmp ( node_generalites -> name,
			       "Numero_devise_totaux_categ" ))
		    no_devise_totaux_categ = my_atoi ( xmlNodeGetContent ( node_generalites ));
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
		node_generalites = node_generalites -> next;
	    }
	}

/* 	s'il y avait un ancien logo mais qu'il n'existe plus, on met le logo par défaut */

	if ( !chemin_logo
	     ||
	     !strlen ( chemin_logo )
	     ||
	     ( chemin_logo
	       &&
	       strlen ( chemin_logo )
	       &&
	       utf8_stat ( chemin_logo, &buffer_stat) == -1 ))
	    chemin_logo = LOGO_PATH;


	/* on recupère ici les comptes et operations */

	if ( !strcmp ( node_1 -> name,
		       "Comptes" ))
	{
	    xmlNodePtr node_comptes;

	    /* node_comptes va faire le tour de l'arborescence des comptes */

	    node_comptes = node_1 -> children;

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
			    p_tab_nom_de_compte_courant = p_tab_nom_de_compte + compte_courant;
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
						   "Nb_operations" ))
					NB_OPE_COMPTE = my_atoi ( xmlNodeGetContent ( node_detail ));

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
						   "Solde_courant" ))
					SOLDE_COURANT = my_strtod ( xmlNodeGetContent ( node_detail ),
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

		    OPERATION_SELECTIONNEE = GINT_TO_POINTER ( -1 );


		    /* on incremente p_tab_nom_de_compte_variable pour le compte suivant */

		    p_tab_nom_de_compte_variable++;
		}

		node_comptes = node_comptes -> next;
	    }
	}

	/* on recupère ici les echeances */

	if ( !strcmp ( node_1 -> name,
		       "Echeances" ))
	{
	    xmlNodePtr node_echeances;

	    /* node_echeances va faire le tour de l'arborescence des echeances */

	    node_echeances = node_1 -> children;

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

		    gsliste_echeances = NULL;

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



			    gsliste_echeances = g_slist_append ( gsliste_echeances,
								 operation_echeance);
			}

			node_detail = node_detail -> next;
		    }
		}
		node_echeances = node_echeances -> next;
	    }
	}

	/* on recupère ici les tiers */

	if ( !strcmp ( node_1 -> name,
		       "Tiers" ))
	{
	    xmlNodePtr node_tiers;

	    /* node_tiers va faire le tour de l'arborescence des tiers */

	    node_tiers = node_1 -> children;

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
	}

	/* on recupère ici les categories */

	if ( !strcmp ( node_1 -> name,
		       "Categories" ))
	{
	    xmlNodePtr node_categories;

	    /* node_categories va faire le tour de l'arborescence des categories */

	    node_categories = node_1 -> children;

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
	    /* creation de la liste des categ pour le combofix */

	    creation_liste_categ_combofix ();
	}

	/* on recupère ici les imputations */

	if ( !strcmp ( node_1 -> name,
		       "Imputations" ))
	{
	    xmlNodePtr node_imputations;

	    /* node_imputations va faire le tour de l'arborescence des imputations */

	    node_imputations = node_1 -> children;

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
	    /* creation de la liste des imputations pour le combofix */

	    creation_liste_imputation_combofix ();
	}


	/* on recupère ici les devises */

	if ( !strcmp ( node_1 -> name,
		       "Devises" ))
	{
	    xmlNodePtr node_devises;

	    /* node_devises va faire le tour de l'arborescence des devises */

	    node_devises = node_1 -> children;

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
	}

	/* on recupère ici les banques */

	if ( !strcmp ( node_1 -> name,
		       "Banques" ))
	{
	    xmlNodePtr node_banques;

	    /* node_banques va faire le tour de l'arborescence des banques */

	    node_banques = node_1 -> children;

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
	}

	/* on recupère ici les exercices */

	if ( !strcmp ( node_1 -> name,
		       "Exercices" ))
	{
	    xmlNodePtr node_exercices;

	    /* node_exercices va faire le tour de l'arborescence des exercices */

	    node_exercices = node_1 -> children;

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
	}

	/* on recupère ici les rapprochements */

	if ( !strcmp ( node_1 -> name,
		       "Rapprochements" ))
	{
	    xmlNodePtr node_rapprochements;

	    /* node_rapprochements va faire le tour de l'arborescence des rapprochements */

	    node_rapprochements = node_1 -> children;

	    while ( node_rapprochements )
	    {
		/* il n'y a pas de generalites ... */

		/* on va recuperer ici les rapprochements */

		if ( !strcmp ( node_rapprochements -> name,
			       "Detail_des_rapprochements" ))
		{
		    xmlNodePtr node_detail;

		    liste_no_rapprochements = NULL;
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

			    liste_no_rapprochements = g_slist_append ( liste_no_rapprochements,
								       rapprochement );
			}
			node_detail = node_detail -> next;
		    }
		}
		node_rapprochements = node_rapprochements -> next;
	    }
	}

	/* on recupère ici les etats */

	if ( !strcmp ( node_1 -> name,
		       "Etats" ))
	{
	    xmlNodePtr node_etats;

	    /* node_etats va faire le tour de l'arborescence des etats */

	    node_etats = node_1 -> children;

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
						       "Aff_verif_ope" ))
					    etat -> afficher_verif_ope = my_atoi ( xmlNodeGetContent ( node_detail_etat ));

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
	}
	node_1 = node_1 -> next;
    }


    /* on libère la memoire */

    xmlFreeDoc ( doc );

    etat.en_train_de_charger = 0;

    /* on marque le fichier comme ouvert */

    fichier_marque_ouvert ( TRUE );

    modification_fichier ( FALSE );

    return ( TRUE );
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* Procedure qui sauvegarde le fichier */
/* retourne TRUE : no problem */
/* retourne FALSE : pas de nom => gtkfileselection a pris la main ou erreur */
/***********************************************************************************************************/

gboolean enregistre_fichier ( gboolean force )
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
    gint nouveau_fichier;

    /*   si le fichier est dejà ouvert par un autre, ne peut enregistrer */

    if ( etat.fichier_deja_ouvert && !etat.force_enregistrement && !force )
    {
	dialogue_conditional_hint ( g_strdup_printf( _("Can not save file \"%s\""), nom_fichier_comptes),

				    g_strdup_printf( _("Grisbi was unable to save this file because it is locked.  Please save it with another name or activate the \"%s\" option in setup.  Alternatively, choose the \"%s\" option below."),
						     _("Force saving of locked files"),
						     _("Do not show this message again")), &(etat.force_enregistrement ) );
	return ( FALSE );
    }

    /* on regarde ici si le fichier existe */
    /*   s'il n'existe pas on mettre ses permissions à 600, sinon on les laisse comme ça */

    if ( utf8_stat ( nom_fichier_comptes,
		&buffer_stat ) == -1 )
	nouveau_fichier = 1;
    else
	nouveau_fichier = 0;


    etat.en_train_de_sauvegarder = 1;

    /* on met à jour les soldes des comptes */

    for ( i=0 ; i<nb_comptes ; i++ )
	mise_a_jour_solde ( i );


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

    /* on met fichier_ouvert à 0 car si c'est une backup ... */
    /* qd c'est un enregistrement normal, la mise à 1 se fait plus tard */

    xmlNewTextChild ( node,
		      NULL,
		      "Fichier_ouvert",
		      "0" );

    if ( force )
	etat.fichier_deja_ouvert = 0;

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
		      "Utilise_exercices",
		      utils_itoa (etat.utilise_exercice));
    xmlNewTextChild ( node,
		      NULL,
		      "Utilise_IB",
		      utils_itoa (etat.utilise_imputation_budgetaire));
    xmlNewTextChild ( node,
		      NULL,
		      "Utilise_PC",
		      utils_itoa (etat.utilise_piece_comptable));
    xmlNewTextChild ( node,
		      NULL,
		      "Utilise_info_BG",
		      utils_itoa (etat.utilise_info_banque_guichet));
    xmlNewTextChild ( node,
		      NULL,
		      "Numero_devise_totaux_tiers",
		      utils_itoa ( no_devise_totaux_tiers));
    xmlNewTextChild ( node,
		      NULL,
		      "Numero_devise_totaux_categ",
		      utils_itoa ( no_devise_totaux_categ));
    xmlNewTextChild ( node,
		      NULL,
		      "Numero_devise_totaux_ib",
		      utils_itoa ( no_devise_totaux_ib));
    xmlNewTextChild ( node,
		      NULL,
		      "Type_affichage_des_echeances",
		      utils_itoa (affichage_echeances));
    xmlNewTextChild ( node,
		      NULL,
		      "Affichage_echeances_perso_nb_libre",
		      utils_itoa (affichage_echeances_perso_nb_libre));
    xmlNewTextChild ( node,
		      NULL,
		      "Type_affichage_perso_echeances",
		      utils_itoa (affichage_echeances_perso_j_m_a));
    xmlNewTextChild ( node,
		      NULL,
		      "Numero_derniere_operation",
		      utils_itoa ( no_derniere_operation));

    xmlNewTextChild ( node,
		      NULL,
		      "Echelle_date_import",
		      utils_itoa (valeur_echelle_recherche_date_import ));

    xmlNewTextChild ( node,
		      NULL,
		      "Utilise_logo",
		      utils_itoa (etat.utilise_logo ));

    xmlNewTextChild ( node,
		      NULL,
		      "Chemin_logo",
		      chemin_logo );

    /* creation de l'ordre de l'affichage des opés */

    pointeur_char = NULL;

    for ( i=0 ; i<4 ; i++ )
	for ( j=0 ; j< 7 ; j++ )
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "-",
					      utils_itoa ( tab_affichage_ope[i][j] ),
					      NULL );
	    else
		pointeur_char = utils_itoa ( tab_affichage_ope[i][j] );


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
					  utils_itoa ( rapport_largeur_colonnes[i] ),
					  NULL );
	else
	    pointeur_char = utils_itoa ( rapport_largeur_colonnes[i] );


    xmlNewTextChild ( node,
		      NULL,
		      "Rapport_largeur_col",
		      pointeur_char );

    /* creation des lignes affichées par caractéristiques */

    xmlNewTextChild ( node,
		      NULL,
		      "Ligne_aff_une_ligne",
		      utils_itoa ( ligne_affichage_une_ligne ));


    pointeur_char = g_strconcat ( utils_itoa ( GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> data )),
				  "-",
				  utils_itoa ( GPOINTER_TO_INT ( lignes_affichage_deux_lignes -> next -> data )),
				  NULL );
    xmlNewTextChild ( node,
		      NULL,
		      "Lignes_aff_deux_lignes",
		      pointeur_char );

    pointeur_char = g_strconcat ( utils_itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> data )),
				  "-",
				  utils_itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> data )),
				  "-",
				  utils_itoa ( GPOINTER_TO_INT ( lignes_affichage_trois_lignes -> next -> next -> data )),
				  NULL );
    xmlNewTextChild ( node,
		      NULL,
		      "Lignes_aff_trois_lignes",
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
					  utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					  NULL );
	else
	    pointeur_char = utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

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
		      utils_itoa ( compte_courant ) );


    /* mise en place des comptes 1 par 1 */

    for ( i=0 ; i < nb_comptes ; i++)
    {
	xmlNodePtr node_compte;

	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	node_1 = xmlNewChild ( node,
			       NULL,
			       "Compte",
			       NULL );

	/* on met d'abord le detail du compte, les types puis les operations */

	/* mise en forme de la date du dernier releve */

	if ( DATE_DERNIER_RELEVE )
	    pointeur_char = g_strconcat ( utils_itoa ( g_date_day ( DATE_DERNIER_RELEVE ) ),
					  "/",
					  utils_itoa ( g_date_month ( DATE_DERNIER_RELEVE ) ),
					  "/",
					  utils_itoa ( g_date_year ( DATE_DERNIER_RELEVE ) ),
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
			  utils_itoa ( NO_COMPTE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Titulaire",
			  TITULAIRE );

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Type_de_compte",
			  utils_itoa ( TYPE_DE_COMPTE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Nb_operations",
			  utils_itoa ( NB_OPE_COMPTE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Devise",
			  utils_itoa ( DEVISE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Banque",
			  utils_itoa ( BANQUE ));

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
			  "Solde_courant",
			  g_strdup_printf ( "%4.7f",
					    SOLDE_COURANT ));

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
			  utils_itoa ( DERNIER_NO_RAPPROCHEMENT ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Compte_cloture",
			  utils_itoa ( COMPTE_CLOTURE ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Affichage_r",
			  utils_itoa ( AFFICHAGE_R ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Nb_lignes_ope",
			  utils_itoa ( NB_LIGNES_OPE ));

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
			  utils_itoa ( g_slist_length ( TYPES_OPES ) ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Type_defaut_debit",
			  utils_itoa ( TYPE_DEFAUT_DEBIT ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Type_defaut_credit",
			  utils_itoa ( TYPE_DEFAUT_CREDIT ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Tri_par_type",
			  utils_itoa ( TRI ));

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Neutres_inclus",
			  utils_itoa ( NEUTRES_INCLUS ));

	/* mise en place de l'ordre du tri */


	pointeur_liste = LISTE_TRI;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_compte,
			  NULL,
			  "Ordre_du_tri",
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
			 utils_itoa ( type -> no_type ));

	    xmlSetProp ( node_type,
			 "Nom",
			 type -> nom_type );

	    xmlSetProp ( node_type,
			 "Signe",
			 utils_itoa ( type -> signe_type ));

	    xmlSetProp ( node_type,
			 "Affiche_entree",
			 utils_itoa ( type -> affiche_entree ));

	    xmlSetProp ( node_type,
			 "Numerotation_auto",
			 utils_itoa ( type -> numerotation_auto ));

	    xmlSetProp ( node_type,
			 "No_en_cours",
			 utils_itoa ( type -> no_en_cours ));

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
			 utils_itoa ( operation -> no_operation ));

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
			 utils_itoa ( operation -> devise ));

	    xmlSetProp ( node_ope,
			 "Rdc",
			 utils_itoa ( operation -> une_devise_compte_egale_x_devise_ope ));

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
			 utils_itoa ( operation -> tiers ));

	    xmlSetProp ( node_ope,
			 "C",
			 utils_itoa ( operation -> categorie ));

	    xmlSetProp ( node_ope,
			 "Sc",
			 utils_itoa ( operation -> sous_categorie ));

	    xmlSetProp ( node_ope,
			 "Ov",
			 utils_itoa ( operation -> operation_ventilee ));

	    xmlSetProp ( node_ope,
			 "N",
			 operation -> notes );

	    xmlSetProp ( node_ope,
			 "Ty",
			 utils_itoa ( operation -> type_ope ));

	    xmlSetProp ( node_ope,
			 "Ct",
			 operation -> contenu_type );

	    xmlSetProp ( node_ope,
			 "P",
			 utils_itoa ( operation -> pointe ));

	    xmlSetProp ( node_ope,
			 "A",
			 utils_itoa ( operation -> auto_man ));

	    xmlSetProp ( node_ope,
			 "R",
			 utils_itoa ( operation -> no_rapprochement ));

	    xmlSetProp ( node_ope,
			 "E",
			 utils_itoa ( operation -> no_exercice ));

	    xmlSetProp ( node_ope,
			 "I",
			 utils_itoa ( operation -> imputation ));

	    xmlSetProp ( node_ope,
			 "Si",
			 utils_itoa ( operation -> sous_imputation ));

	    xmlSetProp ( node_ope,
			 "Pc",
			 operation -> no_piece_comptable );

	    xmlSetProp ( node_ope,
			 "Ibg",
			 operation -> info_banque_guichet );

	    xmlSetProp ( node_ope,
			 "Ro",
			 utils_itoa ( operation -> relation_no_operation ));

	    xmlSetProp ( node_ope,
			 "Rc",
			 utils_itoa ( operation -> relation_no_compte ));

	    xmlSetProp ( node_ope,
			 "Va",
			 utils_itoa ( operation -> no_operation_ventilee_associee ));

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
		      utils_itoa ( nb_echeances ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_derniere_echeance",
		      utils_itoa ( no_derniere_echeance ));

    /* on met les echeances */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_echeances",
			   NULL );

    pointeur_liste = gsliste_echeances;

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
		     utils_itoa ( echeance -> no_operation ));

	xmlSetProp ( node_echeance,
		     "Date",
		     g_strdup_printf ( "%d/%d/%d",
				       echeance -> jour,
				       echeance -> mois,
				       echeance -> annee ));

	xmlSetProp ( node_echeance,
		     "Compte",
		     utils_itoa ( echeance -> compte ));

	xmlSetProp ( node_echeance,
		     "Montant",
		     g_strdup_printf ( "%4.7f",
				       echeance -> montant ));

	xmlSetProp ( node_echeance,
		     "Devise",
		     utils_itoa ( echeance -> devise ));

	xmlSetProp ( node_echeance,
		     "Tiers",
		     utils_itoa ( echeance -> tiers ));

	xmlSetProp ( node_echeance,
		     "Categorie",
		     utils_itoa ( echeance -> categorie ));

	xmlSetProp ( node_echeance,
		     "Sous-categorie",
		     utils_itoa ( echeance -> sous_categorie ));

	xmlSetProp ( node_echeance,
		     "Virement_compte",
		     utils_itoa ( echeance -> compte_virement ));

	xmlSetProp ( node_echeance,
		     "Type",
		     utils_itoa ( echeance -> type_ope ));

	xmlSetProp ( node_echeance,
		     "Type_contre_ope",
		     utils_itoa ( echeance -> type_contre_ope ));

	xmlSetProp ( node_echeance,
		     "Contenu_du_type",
		     echeance -> contenu_type );

	xmlSetProp ( node_echeance,
		     "Exercice",
		     utils_itoa ( echeance -> no_exercice ));

	xmlSetProp ( node_echeance,
		     "Imputation",
		     utils_itoa ( echeance -> imputation ));

	xmlSetProp ( node_echeance,
		     "Sous-imputation",
		     utils_itoa ( echeance -> sous_imputation ));

	xmlSetProp ( node_echeance,
		     "Notes",
		     echeance -> notes );

	xmlSetProp ( node_echeance,
		     "Automatique",
		     utils_itoa ( echeance -> auto_man ));

	xmlSetProp ( node_echeance,
		     "Periodicite",
		     utils_itoa ( echeance -> periodicite ));

	xmlSetProp ( node_echeance,
		     "Intervalle_periodicite",
		     utils_itoa ( echeance -> intervalle_periodicite_personnalisee ));

	xmlSetProp ( node_echeance,
		     "Periodicite_personnalisee",
		     utils_itoa ( echeance -> periodicite_personnalisee ));

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
		     utils_itoa ( echeance -> operation_ventilee ));

	xmlSetProp ( node_echeance,
		     "No_ech_associee",
		     utils_itoa ( echeance -> no_operation_ventilee_associee ));


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
		      utils_itoa ( nb_enregistrements_tiers ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_dernier_tiers",
		      utils_itoa ( no_dernier_tiers ));

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
		     utils_itoa ( tiers -> no_tiers ));

	xmlSetProp ( node_tiers,
		     "Nom",
		     tiers -> nom_tiers );

	xmlSetProp ( node_tiers,
		     "Informations",
		     tiers -> texte );

	xmlSetProp ( node_tiers,
		     "Liaison",
		     utils_itoa ( tiers -> liaison ));

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
		      utils_itoa ( nb_enregistrements_categories ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_derniere_categorie",
		      utils_itoa ( no_derniere_categorie ));

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
		     utils_itoa ( categ -> no_categ ));

	xmlSetProp ( node_categ,
		     "Nom",
		     categ -> nom_categ );

	xmlSetProp ( node_categ,
		     "Type",
		     utils_itoa ( categ -> type_categ ));

	xmlSetProp ( node_categ,
		     "No_derniere_sous_cagegorie",
		     utils_itoa ( categ -> no_derniere_sous_categ ));


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
			 utils_itoa ( sous_categ -> no_sous_categ ));

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
		      utils_itoa ( nb_enregistrements_imputations ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_derniere_imputation",
		      utils_itoa ( no_derniere_imputation ));

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
		     utils_itoa ( imputation -> no_imputation ));

	xmlSetProp ( node_imputation,
		     "Nom",
		     imputation -> nom_imputation );

	xmlSetProp ( node_imputation,
		     "Type",
		     utils_itoa ( imputation -> type_imputation ));

	xmlSetProp ( node_imputation,
		     "No_derniere_sous_imputation",
		     utils_itoa ( imputation -> no_derniere_sous_imputation ));

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
			 utils_itoa ( sous_imputation -> no_sous_imputation ));

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
		      utils_itoa ( nb_devises ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_derniere_devise",
		      utils_itoa ( no_derniere_devise ));

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
		     utils_itoa ( devise -> no_devise ));

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
		     utils_itoa ( devise -> passage_euro ));

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
		     utils_itoa ( devise -> une_devise_1_egale_x_devise_2 ));

	xmlSetProp ( node_devise,
		     "Devise_en_rapport",
		     utils_itoa ( devise -> no_devise_en_rapport ));

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
		      utils_itoa ( nb_banques ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_derniere_banque",
		      utils_itoa ( no_derniere_banque ));

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
		     utils_itoa ( banque -> no_banque ));

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
		      utils_itoa ( nb_exercices ));

    xmlNewTextChild ( node_1,
		      NULL,
		      "No_dernier_exercice",
		      utils_itoa ( no_derniere_exercice ));

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
		     utils_itoa ( exercice -> no_exercice ));

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
		     utils_itoa ( exercice -> affiche_dans_formulaire ));

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
    /* 		    utils_itoa ( g_slist_length ( liste_no_rapprochements ))); */

    /* on met les rapprochements */

    node_1 = xmlNewChild ( node,
			   NULL,
			   "Detail_des_rapprochements",
			   NULL );

    pointeur_liste = liste_no_rapprochements;

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
		     utils_itoa ( rapprochement -> no_rapprochement ));

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
		      utils_itoa ( no_dernier_etat ));

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
			  utils_itoa ( etat -> no_etat ));

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
					      utils_itoa ( GPOINTER_TO_INT ( pointeur_list -> data )),
					      NULL );
	    else
		pointeur_char = utils_itoa ( GPOINTER_TO_INT ( pointeur_list -> data ));

	    pointeur_list = pointeur_list -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Type_classement",
			  pointeur_char );


	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_r",
			  utils_itoa ( etat -> afficher_r ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ope",
			  utils_itoa ( etat -> afficher_opes ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_nb_ope",
			  utils_itoa ( etat -> afficher_nb_opes ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_no_ope",
			  utils_itoa ( etat -> afficher_no_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_date_ope",
			  utils_itoa ( etat -> afficher_date_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_tiers_ope",
			  utils_itoa ( etat -> afficher_tiers_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_categ_ope",
			  utils_itoa ( etat -> afficher_categ_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ss_categ_ope",
			  utils_itoa ( etat -> afficher_sous_categ_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_type_ope",
			  utils_itoa ( etat -> afficher_type_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ib_ope",
			  utils_itoa ( etat -> afficher_ib_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ss_ib_ope",
			  utils_itoa ( etat -> afficher_sous_ib_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_cheque_ope",
			  utils_itoa ( etat -> afficher_cheque_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_notes_ope",
			  utils_itoa ( etat -> afficher_notes_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_pc_ope",
			  utils_itoa ( etat -> afficher_pc_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_verif_ope",
			  utils_itoa ( etat -> afficher_verif_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_rappr_ope",
			  utils_itoa ( etat -> afficher_rappr_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_infobd_ope",
			  utils_itoa ( etat -> afficher_infobd_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_exo_ope",
			  utils_itoa ( etat -> afficher_exo_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Class_ope",
			  utils_itoa ( etat -> type_classement_ope ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_titres_col",
			  utils_itoa ( etat -> afficher_titre_colonnes ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_titres_chgt",
			  utils_itoa ( etat -> type_affichage_titres ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Pas_detail_ventil",
			  utils_itoa ( etat -> pas_detailler_ventilation ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Sep_rev_dep",
			  utils_itoa ( etat -> separer_revenus_depenses ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Devise_gen",
			  utils_itoa ( etat -> devise_de_calcul_general ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Incl_tiers",
			  utils_itoa ( etat -> inclure_dans_tiers ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Ope_click",
			  utils_itoa ( etat -> ope_clickables ));


	xmlNewTextChild ( node_etat,
			  NULL,
			  "Exo_date",
			  utils_itoa ( etat -> exo_date ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_exo",
			  utils_itoa ( etat -> utilise_detail_exo ));

	pointeur_liste = etat -> no_exercices;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_exo",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Plage_date",
			  utils_itoa ( etat -> no_plage_date ));


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
			  utils_itoa ( etat -> separation_par_plage ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Sep_plages",
			  utils_itoa ( etat -> type_separation_plage ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Sep_exo",
			  utils_itoa ( etat -> separation_par_exo ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Deb_sem_plages",
			  utils_itoa ( etat -> jour_debut_semaine ));


	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_comptes",
			  utils_itoa ( etat -> utilise_detail_comptes ));

	pointeur_liste = etat -> no_comptes;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_comptes",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Grp_ope_compte",
			  utils_itoa ( etat -> regroupe_ope_par_compte ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_compte",
			  utils_itoa ( etat -> affiche_sous_total_compte ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_nom_compte",
			  utils_itoa ( etat -> afficher_nom_compte ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Type_vir",
			  utils_itoa ( etat -> type_virement ));

	pointeur_liste = etat -> no_comptes_virements;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_comptes_virements",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Exclure_non_vir",
			  utils_itoa ( etat -> exclure_ope_non_virement ));



	xmlNewTextChild ( node_etat,
			  NULL,
			  "Categ",
			  utils_itoa ( etat -> utilise_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_categ",
			  utils_itoa ( etat -> utilise_detail_categ ));

	pointeur_liste = etat -> no_categ;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_categ",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Exclut_categ",
			  utils_itoa ( etat -> exclure_ope_sans_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_categ",
			  utils_itoa ( etat -> affiche_sous_total_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ss_categ",
			  utils_itoa ( etat -> afficher_sous_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_pas_ss_categ",
			  utils_itoa ( etat -> afficher_pas_de_sous_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_ss_categ",
			  utils_itoa ( etat -> affiche_sous_total_sous_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Devise_categ",
			  utils_itoa ( etat -> devise_de_calcul_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_nom_categ",
			  utils_itoa ( etat -> afficher_nom_categ ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "IB",
			  utils_itoa ( etat -> utilise_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_ib",
			  utils_itoa ( etat -> utilise_detail_ib ));

	pointeur_liste = etat -> no_ib;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_ib",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Exclut_ib",
			  utils_itoa ( etat -> exclure_ope_sans_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_ib",
			  utils_itoa ( etat -> affiche_sous_total_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_ss_ib",
			  utils_itoa ( etat -> afficher_sous_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_pas_ss_ib",
			  utils_itoa ( etat -> afficher_pas_de_sous_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_ss_ib",
			  utils_itoa ( etat -> affiche_sous_total_sous_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Devise_ib",
			  utils_itoa ( etat -> devise_de_calcul_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_nom_ib",
			  utils_itoa ( etat -> afficher_nom_ib ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Tiers",
			  utils_itoa ( etat -> utilise_tiers ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_tiers",
			  utils_itoa ( etat -> utilise_detail_tiers ));

	pointeur_liste = etat -> no_tiers;
	pointeur_char = NULL;

	while ( pointeur_liste )
	{
	    if ( pointeur_char )
		pointeur_char = g_strconcat ( pointeur_char,
					      "/",
					      utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data )),
					      NULL );
	    else
		pointeur_char = utils_itoa ( GPOINTER_TO_INT ( pointeur_liste -> data ));

	    pointeur_liste = pointeur_liste -> next;
	}

	xmlNewTextChild ( node_etat,
			  NULL,
			  "No_tiers",
			  pointeur_char );

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Total_tiers",
			  utils_itoa ( etat -> affiche_sous_total_tiers ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Devise_tiers",
			  utils_itoa ( etat -> devise_de_calcul_tiers ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Aff_nom_tiers",
			  utils_itoa ( etat -> afficher_nom_tiers ));


	xmlNewTextChild ( node_etat,
			  NULL,
			  "Texte",
			  utils_itoa ( etat -> utilise_texte ));

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
			 utils_itoa ( textes_comp -> lien_struct_precedente ));
	    xmlSetProp ( node_3,
			 "Champ",
			 utils_itoa ( textes_comp -> champ ));
	    xmlSetProp ( node_3,
			 "Op",
			 utils_itoa ( textes_comp -> operateur ));
	    xmlSetProp ( node_3,
			 "Txt",
			 textes_comp -> texte );
	    xmlSetProp ( node_3,
			 "Util_txt",
			 utils_itoa ( textes_comp -> utilise_txt ));
	    xmlSetProp ( node_3,
			 "Comp_1",
			 utils_itoa ( textes_comp -> comparateur_1 ));
	    xmlSetProp ( node_3,
			 "Lien_1_2",
			 utils_itoa ( textes_comp -> lien_1_2 ));
	    xmlSetProp ( node_3,
			 "Comp_2",
			 utils_itoa ( textes_comp -> comparateur_2 ));
	    xmlSetProp ( node_3,
			 "Mont_1",
			 utils_itoa ( textes_comp -> montant_1 ));
	    xmlSetProp ( node_3,
			 "Mont_2",
			 utils_itoa ( textes_comp -> montant_2 ));
	    pointeur_liste = pointeur_liste -> next;
	}



	xmlNewTextChild ( node_etat,
			  NULL,
			  "Montant",
			  utils_itoa ( etat -> utilise_montant ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Montant_devise",
			  utils_itoa ( etat -> choix_devise_montant ));

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
			 utils_itoa ( montants_comp -> lien_struct_precedente ));
	    xmlSetProp ( node_3,
			 "Comp_1",
			 utils_itoa ( montants_comp -> comparateur_1 ));
	    xmlSetProp ( node_3,
			 "Lien_1_2",
			 utils_itoa ( montants_comp -> lien_1_2 ));
	    xmlSetProp ( node_3,
			 "Comp_2",
			 utils_itoa ( montants_comp -> comparateur_2 ));
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
			  utils_itoa ( etat -> exclure_montants_nuls ));

	xmlNewTextChild ( node_etat,
			  NULL,
			  "Detail_mod_paie",
			  utils_itoa ( etat -> utilise_mode_paiement ));

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
    resultat = utf8_xmlSaveFormatFile ( nom_fichier_comptes, doc, 1 );

    /* on libère la memoire */

    xmlFreeDoc ( doc );

    if ( resultat == -1 )
    {
	dialogue_error ( g_strdup_printf ( _("Cannot save file '%s': %s"),
					   nom_fichier_comptes, latin2utf8(strerror(errno)) ));
	return ( FALSE );
    }


    /* si c'est un nouveau fichier, on met à 600 ses permissions */

    if ( nouveau_fichier )
	chmod ( nom_fichier_comptes,
		S_IRUSR | S_IWUSR );


    etat.en_train_de_sauvegarder = 0;

    return ( TRUE );
}
/***********************************************************************************************************/



/***********************************************************************************************************/
/* ouvre le fichier, recherche <Fichier_ouvert> */
/* et le met à la valeur demandee */
/***********************************************************************************************************/

void fichier_marque_ouvert ( gint ouvert )
{
    gchar buffer[17];
    gint retour;

    if ( compression_fichier )
	return;

    if ( !nom_fichier_comptes
	 &&
	 !strlen ( nom_fichier_comptes ))
	return;

    /* ouverture du fichier */

#ifdef _WIN32
    // Force binary mode to avoid ftell to take into account unexisting DOS end of lines
    if (!(pointeur_fichier_comptes = utf8_fopen ( nom_fichier_comptes, "r+b")) )
#else
    if (!(pointeur_fichier_comptes = utf8_fopen ( nom_fichier_comptes, "r+")) )
#endif
    {
	dialogue_error ( g_strdup_printf ( _("Cannot lock file '%s': %s"),
					   nom_fichier_comptes,
					   latin2utf8 (strerror (errno)) ));
	return;
    }

    do
	retour = fscanf ( pointeur_fichier_comptes, 
			  "%16s",
			  buffer);
    while ( strcmp ( buffer,
		     "<Fichier_ouvert>" )
	    &&
	    retour != EOF );

    /*   s'il n'a rien trouve, c'est que c'etait la version 0.3.1 et on passe */

    if ( retour != EOF )
    {
	/* CYGWIN et BSD ne pouvant maitriser l'acces bufferise au fichier de
	 * Windows, il gerent 2 curseurs de fichiers differents afin de
	 * maitriser la bufferisation en lecture des fichiers: le
	 * curseur reel au sein du fichier, et un curseur de lecture
	 * simule.  Lors d'un lecture, ils lisent le fichier par
	 * morceaux (le curseur reel est place apres la fin du dernier
	 * morceau lu).  Les fonctions de lecture n'accedent pas en
	 * direct au fichier.  Elles utilisent le curseur de lecture
	 * qu'elles deplacent au sein du buffer memorise. Par contre la
	 * position ce curseur en lecture represente bien la position
	 * reelle du caractere lu au sein du fichier.  (ftell renvoi
	 * cette position correctement) Les fonctions d'ecriture
	 * accedent en direct au fichier, donc elles n'utilisent que le
	 * curseur reel.
	 * 
	 * Pour ecrire apres le dernier caractere lu, il faut donc
	 * repositionner le curseur reel a la positon du curseur de
	 * lecture simule
	 *
	 * -- francois@terrot.net
	 */
	long position = ftell(pointeur_fichier_comptes);
	if ( position != -1 )
	{
	    fseek ( pointeur_fichier_comptes, position, SEEK_SET );
	}
	fprintf ( pointeur_fichier_comptes, utils_itoa ( ouvert ));
    }

    fclose ( pointeur_fichier_comptes );

    return;
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
		     utils_itoa ( categ -> type_categ ));

	xmlSetProp ( node_categ,
		     "No_derniere_sous_cagegorie",
		     utils_itoa ( categ -> no_derniere_sous_categ ));


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
			 utils_itoa ( sous_categ -> no_sous_categ ));

	    xmlSetProp ( node_sous_categ,
			 "Nom",
			 sous_categ -> nom_sous_categ );

	    pointeur_sous_categ = pointeur_sous_categ -> next;
	}
	pointeur_liste = pointeur_liste -> next;
    }


    /* l'arbre est fait, on sauvegarde */

    resultat = utf8_xmlSaveFormatFile ( nom_categ,
				   doc,
				   1 );

    /* on libère la memoire */

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
	    dialogue_warning_hint ( g_strdup_printf (_("This categories list has been produced with grisbi version %s, Grisbi will nevertheless try to import it."), xmlNodeGetContent ( root->children->next->children->next )),
				    _("Version mismatch") );
	  }

	return ( charge_categ_version_0_4_0 ( doc ));
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
    mise_a_jour_categ ();
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
		     utils_itoa ( ib -> type_imputation ));

	xmlSetProp ( node_ib,
		     "No_derniere_sous_imputation",
		     utils_itoa ( ib -> no_derniere_sous_imputation ));


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
			 utils_itoa ( sous_ib -> no_sous_imputation ));

	    xmlSetProp ( node_sous_ib,
			 "Nom",
			 sous_ib -> nom_sous_imputation );

	    pointeur_sous_ib = pointeur_sous_ib -> next;
	}
	pointeur_liste = pointeur_liste -> next;
    }


    /* l'arbre est fait, on sauvegarde */

    resultat = utf8_xmlSaveFormatFile ( nom_ib,
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
    mise_a_jour_imputation ();
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


