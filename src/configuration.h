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

#ifndef __CONFIGURATION_H
#define __CONFIGURATION_H

#include "include.h"


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Structure : ConfigFormat                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Description :                                                           */
/*                                                                          */
/*    Contains the formats to be used for presenting data in grisbi.        */
/*                                                                          */
/*--------------------------------------------------------------------------*/

typedef struct _ConfigFormat {
	gchar  format_date_liste_ope[16]; /* date format in transaction list    */
} ConfigFormat;


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Function : load_config_format                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Description :                                                           */
/*                                                                          */
/*    Reads the "/Configuration/Affichage/Format" elemement of the grisbirc */
/*    file into the "format" part of the configuration.                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Inputs :                                                                */
/*                                                                          */
/*    - node_format: format xml element                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Outputs :                                                               */
/*                                                                          */
/*    - none                                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Returns :                                                               */
/*                                                                          */
/*    - void                                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/

void load_config_format(xmlNodePtr node_format);


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Function : save_config_format                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Description :                                                           */
/*                                                                          */
/*    Write the format section of the configuration to the grisbirc file.   */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Inputs :                                                                */
/*                                                                          */
/*    - parnet_node: parent node of the "Format" element                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Outputs :                                                               */
/*                                                                          */
/*    - none                                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Returns :                                                               */
/*                                                                          */
/*    - void                                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/

#define   DEFAULT_DATE_FORMAT_LISTE_OP  "%d/%m/%Y"

void save_config_format(xmlNodePtr parent_node);


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Function : set_default_config_format                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Description :                                                           */
/*                                                                          */
/*    Initialize the format part of the configuration with default values.  */
/*                                                                          */
/*    - for date format in transaction list: "%d/%M/%Y" (ex. 25/03/2004)    */
/*    - for ...                                                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Inputs :                                                                */
/*                                                                          */
/*    - none                                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Outputs :                                                               */
/*                                                                          */
/*    - none                                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Returns :                                                               */
/*                                                                          */
/*    - void                                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/

void set_default_config_format();


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Function : get_config_format                                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Description :                                                           */
/*                                                                          */
/*    Helper function to get the global configuration for formats.          */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Inputs :                                                                */
/*                                                                          */
/*    - none                                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Outputs :                                                               */
/*                                                                          */
/*    - none                                                                */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Returns :                                                               */
/*                                                                          */
/*    - pointer to the global config format structure.                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/

ConfigFormat *get_config_format();

#endif /* ifndef  __CONFIGURATION_H */
