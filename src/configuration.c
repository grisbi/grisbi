/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2004 Joao F. (joaof@users.sf.net)		              */
/* 			http://www.grisbi.org				                              */
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

#include "configuration.h"
#include "xmlnames.h"

static ConfigFormat  g_config_format;


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Function : load_config_format                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void load_config_format(xmlNodePtr node_format)
{
	xmlNodePtr node;
	for (node = node_format->children ; node ; node = node->next ) 
	{
		if (!strcmp(node -> name, tagAFFICHAGE_FORMAT_DATE_LISTE_OPE)) 
		{
			strcpy(g_config_format.format_date_liste_ope, 
                   xmlNodeGetContent(node));
		}
	}
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Function : save_config_format                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void save_config_format(xmlNodePtr parent_node)
{
	xmlNodePtr format_node = xmlNewChild(parent_node, NULL, tagAFFICHAGE_FORMAT, NULL);

	xmlNewChild(format_node, NULL, tagAFFICHAGE_FORMAT_DATE_LISTE_OPE, g_config_format.format_date_liste_ope);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Function : set_default_config_format                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void set_default_config_format()
{
	strncpy(g_config_format.format_date_liste_ope, 
			DEFAULT_DATE_FORMAT_LISTE_OP, sizeof(g_config_format.format_date_liste_ope)
	);
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Function : get_config_format                                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
ConfigFormat *get_config_format()
{
	return &g_config_format;
}

