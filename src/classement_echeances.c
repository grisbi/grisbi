/* ************************************************************************** */
/*                                  classement_echeances.c                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 CÃ©dric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (aportal@univ-monpt2.fr)	      */
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
#include "classement_echeances.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/




/* ************************************************************************* */
/* Classement de deux Ã©chÃ©ances d'opÃ©rations par date                        */
/* ************************************************************************* */
gint classement_sliste_echeance_par_date ( struct operation_echeance * a, 
					   struct operation_echeance * b )
{
    if ( a->annee > b->annee )
	return 1;
    else if ( a->annee < b->annee )
	return -1;
    else 
    {
	if ( a->mois > b->mois )
	    return 1;
	else if ( a->mois < b->mois )
	    return -1;
	else 
	{
	    if ( a->jour > b->jour )
		return 1;
	    else if ( a->jour < b->jour )
		return -1;
	    else
		return 0;
	}
    }
}
/* ************************************************************************* */



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
