/* ************************************************************************** */
/* work with the struct of method of payment                                  */
/*                                                                            */
/*                                  data_payment                              */
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

/**
 * \file data_payment.c
 * work with the payment structure, no GUI here
 */


#include "include.h"


/*START_INCLUDE*/
#include "data_payment.h"
#include "comptes_traitements.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/** create the g_slist of the default method of payment
 * \param none
 * \return a g_slist with the struct of method of payment
 * */
GSList *gsb_payment_default_payment_list ( void )
{

    printf ( "faire le défaut des types de paiements\n" );
    /*     FIXME : cf creation_types_par_defaut */

    return NULL;
}

