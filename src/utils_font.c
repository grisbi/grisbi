/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	          */
/*          2008-2017 Pierre Biava (grisbi@pierre.biava.name)                 */
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

/**
 * \file gsb_utils_font.c
 * this file contains usefull functions to work with fonts
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "utils_font.h"
#include "grisbi_win.h"
#include "utils_str.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean utils_font_choose (GtkWidget *button,
				    gchar **fontname);
static void utils_font_update_labels (GtkWidget *button,
				       const gchar *fontname);
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/




