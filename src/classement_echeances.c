/* ************************************************************************** */
/*                                  classement_echeances.c                    */
/*                                                                            */
/*     Copyright (C)    2000-2003 CÃ©dric Auger (cedric@grisbi.org)           */
/*          2003-2004 Benjamin Drieu (bdrieu@april.org)                       */
/*          2003-2004 Alain Portal (aportal@univ-montp2.fr)                   */
/*          http://www.grisbi.org                                             */
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
#include "./gsb_data_scheduled.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/




/* ************************************************************************* */
/* Classement de deux échéances d'opérations par date                        */
/* ************************************************************************* */
gint classement_sliste_echeance_par_date ( gint scheduled_number_1, 
					   gint scheduled_number_2 )
{
    return ( g_date_compare ( gsb_data_scheduled_get_date (scheduled_number_1),
			      gsb_data_scheduled_get_date (scheduled_number_2) ));
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
