/* ************************************************************************** */
/*                                  utils_types.c                             */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-montp2.fr)	      */
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
#include "utils_types.h"
#include "search_glist.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
/*END_EXTERN*/





/* ************************************************************************************************************** */
/* renvoie le type ope demandÃ© en argument */
/* ou NULL si pas trouvÃ© */
/* ************************************************************************************************************** */
struct struct_type_ope *type_ope_par_no ( gint no_type_ope,
					  gint no_compte )
{
    GSList *liste_tmp;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

    liste_tmp = g_slist_find_custom ( gsb_account_get_method_payment_list (no_compte),
				      GINT_TO_POINTER ( no_type_ope ),
				      (GCompareFunc) recherche_type_ope_par_no );

    if ( liste_tmp )
	return ( liste_tmp -> data );

    return NULL;
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
/* renvoie le nom du type_ope correspondant au numÃ©ro donnÃ© */
/* ou null */
/* ************************************************************************************************************** */
gchar *type_ope_name_by_no ( gint no_type_ope,
			     gint no_de_compte )
{
    struct struct_type_ope *type_ope;

    type_ope = type_ope_par_no ( no_type_ope,
				 no_de_compte );

    if ( type_ope )
	return ( type_ope -> nom_type );

    return NULL;
}
/* ************************************************************************************************************** */


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
