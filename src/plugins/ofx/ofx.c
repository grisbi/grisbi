/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2006-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

/*START_STATIC*/
/*END_STATIC*/



/** Module name. */
G_MODULE_EXPORT const gchar plugin_name[] = "ofx";



/** Initialization function. */
G_MODULE_EXPORT void plugin_register () {
    printf ("Initializating ofx plugin\n");
}



/** Main function of module. */
G_MODULE_EXPORT gint plugin_run ( gchar * file_name, gchar **file_content,
				  gboolean crypt, gulong length )
{
    return gsb_file_util_crypt_file ( file_name, file_content, crypt, length );    
}



/** Release plugin  */
G_MODULE_EXPORT gboolean plugin_release ( )
{
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
