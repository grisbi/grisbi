/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	     2004 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "metatree.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/



/**
 *
 *
 */
gboolean metatree_get_row_properties ( GtkTreeModel * tree_model, GtkTreePath * path, 
				       gchar ** text, gint * lvl1, gint * lvl2, 
				       gpointer * data )
{
    GtkTreeIter iter;
    gint tmp_lvl1, tmp_lvl2;
    gpointer tmp_data;
    gchar * tmp_text;

    if ( !gtk_tree_model_get_iter ( GTK_TREE_MODEL(tree_model), &iter, path ) )
    {
	/* This can be because drag is not possible, so no croak */
	return FALSE;
    }

    gtk_tree_model_get ( GTK_TREE_MODEL(tree_model), &iter,
			 META_TREE_TEXT_COLUMN, &tmp_text,
			 META_TREE_NO_DIV_COLUMN, &tmp_lvl1,
			 META_TREE_NO_SUB_DIV_COLUMN, &tmp_lvl2,
			 META_TREE_POINTER_COLUMN, &tmp_data,
			 -1);

    if ( text ) *text = tmp_text;
    if ( lvl1 ) *lvl1 = tmp_lvl1;
    if ( lvl2 ) *lvl2 = tmp_lvl2;
    if ( data ) *data = tmp_data;

    return TRUE;
}



/**
 *
 *
 */
enum meta_tree_row_type metatree_get_row_type ( GtkTreeModel * tree_model, 
						GtkTreePath * path )
{
    gint lvl1, lvl2;
  
    if ( metatree_get_row_properties ( tree_model, path, NULL, &lvl1, &lvl2, NULL ) )
    {
	if ( lvl1 == -1 )
	{
	    if ( lvl2 == -1 )
		return META_TREE_TRANSACTION;
	    else 
		return META_TREE_INVALID;
	}
	else
	{
	    if ( lvl2 == -1 )
		return META_TREE_DIV;
	    else
		return META_TREE_SUB_DIV;
	}
    }

    return META_TREE_INVALID;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
