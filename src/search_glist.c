/* ************************************************************************** */
/*   									      */
/*  					                                      */
/*                                                                            */
/*                                  search_glist.c                            */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
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
#include "search_glist.h"
#include "gsb_data_transaction.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/




/*START_EXTERN*/
/*END_EXTERN*/






/******************************************************************************/
gint recherche_rapprochement_par_nom ( struct struct_no_rapprochement *rapprochement,
				       gchar *no_rap )
{
    return ( strcmp ( rapprochement -> nom_rapprochement,
		      no_rap ));
}
/******************************************************************************/

/******************************************************************************/
gint recherche_rapprochement_par_no ( struct struct_no_rapprochement *rapprochement,
				      gint *no_rap )
{
    return ( !(rapprochement -> no_rapprochement == GPOINTER_TO_INT ( no_rap )));
}
/******************************************************************************/







/* *******************************************************************************/
gint recherche_operation_par_id ( gpointer operation,
				  gchar *id_recherchee )
{
    gint transaction_number;
    
    /* FIXME : à mettre dans gsb_transaction_data.c */
    
    transaction_number = gsb_data_transaction_get_transaction_number (operation);

    if ( gsb_data_transaction_get_transaction_id ( transaction_number ))
	return ( strcmp ( id_recherchee,
			  gsb_data_transaction_get_transaction_id ( transaction_number)));
    else
	return -1;
}
/* *******************************************************************************/



/* ************************************************************************************************************** */
gint recherche_type_ope_par_no ( struct struct_type_ope *type_ope,
				 gint *no_type )
{
    return ( !(type_ope->no_type == GPOINTER_TO_INT(no_type)) );
}
/* ************************************************************************************************************** */


/* ************************************************************************************************************** */
gint cherche_string_equivalente_dans_slist ( gchar *string_list,
					     gchar *string_cmp )
{
    return ( strcmp ( string_list,
		      string_cmp ));
}
/* ************************************************************************************************************** */


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
