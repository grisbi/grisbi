/* ************************************************************************** */
/*                                  utils_tiers.c                             */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
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
#include "utils_tiers.h"
#include "tiers_onglet.h"
#include "search_glist.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern GSList *liste_struct_tiers;
/*END_EXTERN*/






/* **************************************************************************************************** */
/* renvoie le tiers demandé par son no */
/* ou NULL si pas trouvé */
/* **************************************************************************************************** */
struct struct_tiers *tiers_par_no ( gint no_tiers )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_tiers,
				      GINT_TO_POINTER ( no_tiers ),
				      (GCompareFunc) recherche_tiers_par_no );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* renvoie le tiers demandé par son no */
/* si creer = 1, crée le tiers si pas trouvé */
/* ou NULL si pb */
/* **************************************************************************************************** */
struct struct_tiers *tiers_par_nom ( gchar *nom_tiers,
				     gboolean creer )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_tiers,
				      g_strstrip ( nom_tiers ),
				      (GCompareFunc) recherche_tiers_par_nom );

    if ( liste_tmp )
	return ( liste_tmp -> data );
    else
    {
	if ( creer )
	{
	    struct struct_tiers *tiers;

	    tiers = ajoute_nouveau_tiers ( nom_tiers );
	    return ( tiers );
	}
    }

    return NULL;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* retourne le tiers en donnant comme argument son numéro */
/* retour : soit le nom du tiers
 * 	    soit No third party defined si return_null est FALSE et pas de tiers trouvé,
 * 	    soit NULL si return_null est TRUE et pas de tiers trouvé */
/* **************************************************************************************************** */

gchar *tiers_name_by_no ( gint no_tiers,
			  gboolean return_null )
{
    struct struct_tiers *tiers;

    tiers = tiers_par_no ( no_tiers );

    if (tiers)
	return ( tiers->nom_tiers );
    else
	if ( return_null )
	    return NULL;
	else
	    return ( g_strdup (_("No third party defined")));
}
/* **************************************************************************************************** */

