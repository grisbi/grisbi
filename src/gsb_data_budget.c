/* ************************************************************************** */
/* work with the struct of budget                                             */
/*                                                                            */
/*                                                                            */
/*     Copyright (C)	2000-2005 Cédric Auger (cedric@grisbi.org)	      */
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
 * \file gsb_budget_data.c
 * work with the budget structure, no GUI here
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_budget.h"
#include "utils_ib.h"
#include "structures.h"
/*END_INCLUDE*/


/**
 * \struct 
 * Describe a budget
 */
/* typedef struct */
/* { */
/*  */
/* } struct_transaction; */


/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * merge the given budget list with the current budget list
 *
 * \param list_to_merge
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_budget_merge_budget_list ( GSList *list_to_merge )
{
    GSList *list_tmp;

    list_tmp = list_to_merge;

    while ( list_tmp )
    {
	struct struct_imputation *new_budget;
	struct struct_imputation *last_budget;
	
	new_budget = list_tmp -> data;

	/* we try to find the new budget in the currents budgets
	 * if don't, it creates it */

	last_budget = imputation_par_nom ( new_budget -> nom_imputation,
					   TRUE,
					   new_budget -> type_imputation,
					   0 );

	/* here normally last budget is real, we can append the sub-budget */

	if ( last_budget )
	{
	    GSList *sub_list_tmp;

	    sub_list_tmp = new_budget -> liste_sous_imputation;

	    while ( sub_list_tmp )
	    {
		struct struct_sous_imputation *new_sub_budget;

		new_sub_budget = sub_list_tmp -> data;

		sous_imputation_par_nom ( last_budget,
					  new_sub_budget -> nom_sous_imputation,
					  TRUE );

		sub_list_tmp = sub_list_tmp -> next;
	    }
	}
	list_tmp = list_tmp -> next;
    }

    return TRUE;
}


