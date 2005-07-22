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
#include "utils_str.h"
#include "traitement_variables.h"
#include "search_glist.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean charge_ib_version_0_4_0 ( xmlDocPtr doc );
/*END_STATIC*/


/* used to get the sort of the accounts in the version < 0.6 */
static GSList *sort_accounts;

gchar *nom_fichier_comptes;

gchar *titre_fichier;
gchar *adresse_commune;
gchar *adresse_secondaire;


/*START_EXTERN*/
extern GSList *liste_struct_categories;
extern GSList *liste_struct_imputation;
extern gint nb_enregistrements_categories, no_derniere_categorie;
extern gint nb_enregistrements_imputations;
extern gint nb_enregistrements_categories, no_derniere_categorie;
extern gint no_derniere_imputation;
/*END_EXTERN*/





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
			ib -> type_imputation = utils_str_atoi ( latin2utf8(xmlGetProp ( node_detail,
									       "Type" )));
			ib -> no_derniere_sous_imputation = utils_str_atoi ( latin2utf8(xmlGetProp ( node_detail,
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

				sous_ib -> no_sous_imputation = utils_str_atoi ( latin2utf8(xmlGetProp ( node_sous_ib,
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





/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
