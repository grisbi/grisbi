/* ************************************************************************** */
/*                                  utils_rapprochements.c                    */
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
#include "utils_rapprochements.h"
#include "search_glist.h"
/*END_INCLUDE*/

/*START_STATIC*/
static struct struct_no_rapprochement *rapprochement_par_no ( gint no_rapprochement );
/*END_STATIC*/


/*START_EXTERN*/
extern GSList *liste_struct_rapprochements;
/*END_EXTERN*/





/* ********************************************************************************************************* */
/* renvoie l'adr l'adr du rapprochement demandÃ© par son no */
/* ou NULL si pas trouvÃ© */
/* ********************************************************************************************************* */
struct struct_no_rapprochement *rapprochement_par_no ( gint no_rapprochement )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_rapprochements,
				      GINT_TO_POINTER ( no_rapprochement ),
				      (GCompareFunc) recherche_rapprochement_par_no );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/* ********************************************************************************************************* */



/* ********************************************************************************************************* */
/* renvoie l'adr l'adr du rapprochement demandÃ© par son nom */
/* ou NULL si pas trouvÃ© */
/* ********************************************************************************************************* */
struct struct_no_rapprochement *rapprochement_par_nom ( gchar *nom_rapprochement )
{
    GSList *liste_tmp;

    liste_tmp = g_slist_find_custom ( liste_struct_rapprochements,
				      g_strstrip ( nom_rapprochement ),
				      (GCompareFunc) recherche_rapprochement_par_nom );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/* ********************************************************************************************************* */



/* ********************************************************************************************************* */
/* renvoie le nom du rapprochement ou NULL s'il n'existe pas */
/* ********************************************************************************************************* */
gchar *rapprochement_name_by_no ( gint no_rapprochement )
{
    struct struct_no_rapprochement *rapprochement;

    rapprochement = rapprochement_par_no ( no_rapprochement );

    if ( rapprochement )
	return ( g_strdup (rapprochement -> nom_rapprochement ));

    return NULL;
}
/* ********************************************************************************************************* */




