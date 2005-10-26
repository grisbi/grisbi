/* ************************************************************************** */
/*                                  utils_xml.c				      */
/*                                                                            */
/*     Copyright (C)	2004     Benjamin Drieu (bdrieu@april.org)	      */
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
#include <libxml/parser.h>


/*START_INCLUDE*/
#include "utils_xml.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/



gboolean node_strcmp ( xmlNodePtr node, gchar * name )
{
  return node -> name && !strcmp ( node -> name, name );
}



xmlNodePtr get_child ( xmlNodePtr node, gchar * child_name )
{
  xmlNodePtr iter_node = node -> children;

  if (!node) return NULL;

  while ( iter_node )
    {
      if ( node_strcmp(iter_node, child_name) )
	return iter_node;

      iter_node = iter_node -> next;
    }

  return NULL;
}


gchar * child_content ( xmlNodePtr node, gchar * child_name )
{
  xmlNodePtr child_node;

  if (!node) return NULL;

  child_node = get_child ( node, child_name );

  if ( child_node )
    return (gchar *) xmlNodeGetContent ( child_node );
  
  return NULL;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
