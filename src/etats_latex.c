/*  Fichier qui s'occupe d'afficher les états via une latex */
/*      etats_latex.c */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)		      */
/* 			http://www.grisbi.org				      */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */



#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "en_tete.h"

#include "etats_latex.h"

enum alignement {
  LEFT, CENTER, RIGHT,
};


/**
 * FIXME: TODO
 *
 */
void latex_attach_label ( gchar * text, int x, int x2, int y, int y2, 
			     enum alignement align, struct structure_operation * ope )
{
  printf ( "%s\n", text );
}



/**
 * FIXME: TODO
 *
 */
void latex_attach_vsep ( int x, int x2, int y, int y2)
{
  printf ( "|\n", text );
}



/**
 * FIXME: TODO
 *
 */
void latex_attach_hsep ( int x, int x2, int y, int y2)
{
  printf ( "-\n", text );
}



gint latex_initialise (GSList * opes_selectionnees)
{
  printf (">>> Initialisation\n");

  return 1;
}



gint latex_finish ()
{
  printf (">>> Fin\n");

  return 1;
}

