/* ************************************************************************** */
/*                                  utils_echeances.c                         */
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
#include "utils_echeances.h"
#include "search_glist.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern GSList *liste_struct_echeances;
/*END_EXTERN*/







/*****************************************************************************************************/
/* renvoie l'adr de l'Ã©chÃ©ance demandÃ©e par son no */
/* renvoie NULL si pas trouvÃ©e */
/*****************************************************************************************************/
struct operation_echeance *echeance_par_no ( gint no_echeance )
{
   if ( no_echeance )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_echeances,
					  GINT_TO_POINTER ( no_echeance ),
					  (GCompareFunc) recherche_echeance_par_no );

	if ( liste_tmp )
	    return ( liste_tmp -> data );
    }
    return NULL;
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* renvoie l'adr de l'Ã©chÃ©ance demandÃ©e par son no de banque */
/* renvoie NULL si pas trouvÃ©e */
/*****************************************************************************************************/
struct operation_echeance *echeance_par_no_compte ( gint no_compte )
{
   if ( no_compte )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_echeances,
					  GINT_TO_POINTER ( no_compte ),
					  (GCompareFunc) recherche_echeance_par_no_compte );

	if ( liste_tmp )
	    return ( liste_tmp -> data );
    }
    return NULL;
}
/*****************************************************************************************************/

