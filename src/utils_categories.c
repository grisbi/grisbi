/* ************************************************************************** */
/*                                  utils_categories.c                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (dionysos@grisbi.org)	      */
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

#include "include.h"


/*START_INCLUDE*/
#include "utils_categories.h"
#include "search_glist.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern GSList *liste_struct_categories;
extern gint mise_a_jour_combofix_categ_necessaire;
extern gint nb_enregistrements_categories;
extern gint no_derniere_categorie;
/*END_EXTERN*/







/* **************************************************************************************************** */
/* retourne l'adr de la categ dont le nom est donnÃ© en argument */
/* si pas trouvÃ©e : */
/* renvoie NULL si creer = 0*/
/* la crÃ©e et renvoie son adr avec le type_categ et la derniere sous_categ donnÃ©s si creer = 1 */
/* type_categ = 0=crÃ©dit ; 1 = dÃ©bit ; 2 = spÃ©cial */
/* si on ajoute une categ, on met mise_a_jour_combofix_categ_necessaire Ã  1 */
/* **************************************************************************************************** */

struct struct_categ *categ_par_nom ( gchar *nom_categ,
				     gboolean creer,
				     gint type_categ,
				     gint no_derniere_sous_categ )
{
    if ( nom_categ
	 &&
	 strlen (g_strstrip ( nom_categ )))
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_categories,
					  g_strstrip (nom_categ),
					  (GCompareFunc) recherche_categorie_par_nom );

	if ( liste_tmp )
	    return liste_tmp -> data;
	else
	{
	    if ( creer )
	    {
		struct struct_categ *nouvelle_categorie;

		nouvelle_categorie = calloc ( 1,
					      sizeof ( struct struct_categ ));

		nouvelle_categorie -> no_categ = ++no_derniere_categorie;
		nouvelle_categorie -> nom_categ = g_strdup ( g_strstrip ( nom_categ ));
		nouvelle_categorie -> type_categ = type_categ;
		nouvelle_categorie -> no_derniere_sous_categ = no_derniere_sous_categ;

		liste_struct_categories = g_slist_append ( liste_struct_categories,
							   nouvelle_categorie );
		nb_enregistrements_categories++;
		mise_a_jour_combofix_categ_necessaire = 1;

		return ( nouvelle_categorie );
	    }
	}
    }
    return NULL;
}
/* **************************************************************************************************** */
   



/* **************************************************************************************************** */
/* retourne l'adr de la sous categ dont le nom est donnÃ© en argument */
/* si pas trouvÃ©e : */
/* la crÃ©e et renvoie son adr si creer=1 */
/* renvoie NULL si creer = 0 */
/* si on ajoute une categ, on met mise_a_jour_combofix_categ_necessaire Ã  1 */
/* **************************************************************************************************** */

struct struct_sous_categ *sous_categ_par_nom ( struct struct_categ *categ,
					       gchar *nom_sous_categ,
					       gboolean creer )
{
    if ( categ
	 &&
	 nom_sous_categ
	 &&
	 strlen ( g_strstrip ( nom_sous_categ )))
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( categ -> liste_sous_categ,
					  g_strstrip (nom_sous_categ),
					  (GCompareFunc) recherche_sous_categorie_par_nom );

	if ( liste_tmp )
	    return liste_tmp -> data;
	else
	{
	    if ( creer )
	    {
		struct struct_sous_categ *nouvelle_sous_categorie;

		nouvelle_sous_categorie = malloc ( sizeof ( struct struct_sous_categ ));

		nouvelle_sous_categorie -> no_sous_categ = ++( categ -> no_derniere_sous_categ );
		nouvelle_sous_categorie -> nom_sous_categ = g_strdup ( g_strstrip ( nom_sous_categ ));

		categ -> liste_sous_categ = g_slist_append ( categ -> liste_sous_categ,
							     nouvelle_sous_categorie );

		mise_a_jour_combofix_categ_necessaire = 1;
		return ( nouvelle_sous_categorie );
	    }
	}
    }

    return NULL;
}
/* **************************************************************************************************** */





/* **************************************************************************************************** */
/* cette fonction renvoie l'adr de la categ demandÃ©e en argument */
/* et NULL si pas trouvÃ©e */
/* **************************************************************************************************** */
struct struct_categ *categ_par_no ( gint no_categorie )
{
    if ( no_categorie )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_categories,
					  GINT_TO_POINTER ( no_categorie ),
					  (GCompareFunc) recherche_categorie_par_no );

	if ( liste_tmp )
	    return ( liste_tmp -> data );
    }
    return NULL;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* cette fonction renvoie l'adr de la sous categ demandÃ©e */
/* ou NULL si la categ ou la sous categ n'existe pas */
/* **************************************************************************************************** */
struct struct_sous_categ *sous_categ_par_no ( gint no_categorie,
					      gint no_sous_categorie )
{
    if ( no_categorie
	 &&
	 no_sous_categorie )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_categories,
					  GINT_TO_POINTER ( no_categorie ),
					  (GCompareFunc) recherche_categorie_par_no );

	if ( liste_tmp )
	{
	    struct struct_categ *categ;

	    categ = liste_tmp -> data;

	    liste_tmp = g_slist_find_custom ( categ -> liste_sous_categ,
					      GINT_TO_POINTER ( no_sous_categorie ),
					      (GCompareFunc) recherche_sous_categorie_par_no );

	    if ( liste_tmp )
		return ( liste_tmp -> data );
	}
    }
    return NULL;
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* cette fonction renvoie une chaine de la forme */
/* soit categ : sous categ */
/* soit categ si no_sous_categorie=0 ou si la sous_categ n'existe pas */
/* ou NULL si la categ n'existe pas */
/* **************************************************************************************************** */
gchar *nom_categ_par_no ( gint no_categorie,
			  gint no_sous_categorie )
{
    struct struct_categ *categ;
    struct struct_sous_categ *sous_categ;

    categ = categ_par_no ( no_categorie );
    sous_categ = sous_categ_par_no ( no_categorie,
				     no_sous_categorie );

    if ( sous_categ )
	/* 	s'il y a une sous categ, c'est qu'il y a une categ... */
	return ( g_strconcat ( categ -> nom_categ,
			       " : ",
			       sous_categ -> nom_sous_categ,
			       NULL ));
    else
	if ( categ )
	    return ( g_strdup ( categ -> nom_categ ));
	
    return NULL;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* cette fonction renvoie une chaine contenant la sous categ */
/* ou NULL si la categ ou la sous categ n'existe pas */
/* **************************************************************************************************** */
gchar *nom_sous_categ_par_no ( gint no_categorie,
			       gint no_sous_categorie )
{
    struct struct_sous_categ *sous_categ;

    sous_categ = sous_categ_par_no ( no_categorie,
				     no_sous_categorie );

    if ( sous_categ )
	    return ( g_strdup ( sous_categ -> nom_sous_categ ));
	
    return NULL;
}
/* **************************************************************************************************** */


