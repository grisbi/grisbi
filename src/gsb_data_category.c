/* ************************************************************************** */
/* work with the struct of categories                                         */
/*                                                                            */
/*                                  data_account                              */
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
 * \file gsb_category_data.c
 * work with the category structure, no GUI here
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_data_category.h"
#include "utils_categories.h"
#include "structures.h"
/*END_INCLUDE*/



/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/


/**
 * merge the given category list with the current category list
 *
 * \param list_to_merge
 *
 * \return TRUE if ok
 * */
gboolean gsb_data_category_merge_category_list ( GSList *list_to_merge )
{
    GSList *list_tmp;

    list_tmp = list_to_merge;

    while ( list_tmp )
    {
	struct struct_categ *new_category;
	struct struct_categ *last_category;
	
	new_category = list_tmp -> data;

	/* we try to find the new category in the currents categories
	 * if don't, it creates it */

	last_category = categ_par_nom ( new_category -> nom_categ,
					TRUE,
					new_category -> type_categ,
					0 );

	/* here normally last category is real, we can append the sub-categories */

	if ( last_category )
	{
	    GSList *sub_list_tmp;

	    sub_list_tmp = new_category -> liste_sous_categ;

	    while ( sub_list_tmp )
	    {
		struct struct_sous_categ * new_sub_category;

		new_sub_category = sub_list_tmp -> data;

		sous_categ_par_nom ( last_category,
				     new_sub_category -> nom_sous_categ,
				     TRUE );

		sub_list_tmp = sub_list_tmp -> next;
	    }
	}
	list_tmp = list_tmp -> next;
    }

    return TRUE;
}


