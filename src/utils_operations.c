/* ************************************************************************** */
/*                                  utils_operations.c                    */
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
#include "utils_operations.h"
#include "search_glist.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
/*END_EXTERN*/






/*****************************************************************************************************/
/* renvoie l'adr de l'opÃ© demandÃ©e par son no */
/* ou NULL si pas trouvÃ©e */
/*****************************************************************************************************/
struct structure_operation *operation_par_no ( gint no_operation,
					       gint no_compte )
{
    GSList *liste_tmp;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;
    
    liste_tmp = g_slist_find_custom ( LISTE_OPERATIONS,
				      GINT_TO_POINTER ( no_operation ),
				      (GCompareFunc) recherche_operation_par_no );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* renvoie l'adr de l'opÃ© demandÃ©e par son no de cheque */
/* ou NULL si pas trouvÃ©e */
/*****************************************************************************************************/
struct structure_operation *operation_par_cheque ( gint no_cheque,
						   gint no_compte )
{
    GSList *liste_tmp;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;
    
    liste_tmp = g_slist_find_custom ( LISTE_OPERATIONS,
				      GINT_TO_POINTER ( no_cheque ),
				      (GCompareFunc) recherche_operation_par_cheque );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* renvoie l'adr de l'opÃ© demandÃ©e par son no id*/
/* ou NULL si pas trouvÃ©e */
/*****************************************************************************************************/
struct structure_operation *operation_par_id ( gchar *no_id,
					       gint no_compte )
{
    GSList *liste_tmp;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;
    
    liste_tmp = g_slist_find_custom ( LISTE_OPERATIONS,
				      g_strstrip ( no_id ),
				      (GCompareFunc) recherche_operation_par_id );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/*****************************************************************************************************/




