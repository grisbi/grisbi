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


/* Definition of data formatting functions (date, etc.) */

#ifndef __GRISBI_FORMAT_H
#define __GRISBI_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include  <glib.h>
/* #include  "include.h" */

  	
	/* Date formatting */
  
/*--------------------------------------------------------------------------*/
/* some predefined formats (NULL terminated array)                          */
/* (to be used to provide the user with a list of choices for date format)  */
/*--------------------------------------------------------------------------*/

/* extern gchar *g_predefined_formats[]; */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Function : gsb_format_gdate                                             */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Description :                                                           */
/*                                                                          */
/*    Format a GDate according the given format.                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Inputs :                                                                */
/*                                                                          */
/*    - gdate: the date to be formatted                                     */
/*    - format: the format to use (see strftime)                            */
/*    - len: size of buffer                                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Outputs :                                                               */
/*                                                                          */
/*    - buf: pointer to the buffer containing the formatted date            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  Returns :                                                               */
/*                                                                          */
/*    - the formatted string (buf)                                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/

    /* START_INCLUDE_H */
    /* END_INCLUDE_H */


/*START_DECLARATION*/
gchar *gsb_format_gdate(GDate *gdate);
/*END_DECLARATION*/


#endif /* ifndef __GRISBI_FORMAT_H */
