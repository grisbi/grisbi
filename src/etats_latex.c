/*  Fichier qui s'occupe d'afficher les états via une latex */
/*      etats_latex.c */

/*     Copyright (C)	2004 Benjamin Drieu (bdrieu@april.org)		      */
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

int lastline = 0;
int last_is_hsep = 0;
FILE * out;


struct struct_etat_affichage latex_affichage = {
  latex_initialise,
  latex_finish,
  latex_attach_hsep,
  latex_attach_vsep,
  latex_attach_label,
};


/**
 * FIXME: TODO
 *
 */
void latex_attach_label ( gchar * text, int x, int x2, int y, int y2, 
			  enum alignement align, struct structure_operation * ope )
{
  gchar * safe_text;

  if ( !text )
    text = "";

  fprintf ( out, "%% %d,%d -> %d,%d : %s\n", x, x2, y, y2, text );

  if ( y >= lastline )
    {
      lastline = y2;
      if ( ! last_is_hsep )
	{
	  int pad;
	  fprintf ( out, "\\\\\n" );
	  for ( pad = 0 ; pad < x ; pad ++ )
	    latex_attach_label ( NULL, pad, pad+1, y, y2, align, ope );
	}
    }
  else
    {
      fprintf ( out, "&{\\vrule}&", text );
    }


  fprintf ( out, "\\begin{minipage}{\\hsize}\n" );

  switch ( align )
    {
    case LEFT:
      fprintf ( out, "\\raggedright\n" );
      break;
  
    case RIGHT:
      fprintf ( out, "\\raggedleft\n" );
      break;
  
    case CENTER:
      break;
    }

  safe_text = latex_safe(text);
  fprintf ( out, "%s\n\\end{minipage}", safe_text );
  free ( safe_text );

/*   if ( x2 < nb_colonnes ) */
/*     fprintf ( out, "&{\\vrule}&", text ); */

  last_is_hsep = 0;
}



/**
 * FIXME: TODO
 *
 */
void latex_attach_vsep ( int x, int x2, int y, int y2)
{
  if ( y >= lastline )
    {
      if ( ! last_is_hsep )
	fprintf ( out, "\\\\\n" );
      lastline = y2;
    }

  fprintf ( out, "{\\vrule}&" );

  last_is_hsep = 0;
}



/**
 * FIXME: TODO
 *
 */
void latex_attach_hsep ( int x, int x2, int y, int y2)
{
/*   if ( y > lastline ) */
/*     {       */
      if ( ! last_is_hsep )
	fprintf ( out, "\\\\\n" );
      lastline = y2;
/*     } */
  fprintf ( out, "\\hline\n" );

  last_is_hsep = 1;
}



gint latex_initialise (GSList * opes_selectionnees)
{
  int i;

  out = fopen ("toto.tex", "w");
  if (! out)
    {
      fprintf ( stderr, _("Cannot open file '%s': %s\n"), "toto.tex", 
		strerror(errno) );
      return FALSE;
     }

  fprintf (out, 
	   "\\documentclass{article}\n\n"
	   "\\usepackage{a4}\n"
	   "\\usepackage[utf8]{inputenc}\n"
	   "\\usepackage{eurosym}\n"
	   "\\usepackage{landscape}\n"
	   "\\usepackage{longtable}\n"
	   "\\begin{document}\n\n"
	   "\\begin{longtable}[l]{");

  for (i = 1 ; i < nb_colonnes ; i++)
    {
      fprintf (out, "p{3cm}c");
    }

  fprintf (out, "p{3cm}}\n");

  return 1;
}



gint latex_finish ()
{
  fprintf (out, "\\end{longtable}\n"
	   "\\end{document}\n");

  fclose (out);

  if ( system ( "latex -interaction=nonstopmode toto.tex" ) > 0 )
    dialogue ( _("LaTeX run was unable to complete, see console output for details.") );
  else 
    system ( "xdvi toto.dvi &" );
  
  return 1;
}



gchar * latex_safe ( gchar * text ) {
  gchar * safe_text, *tmp;

  if ( ! text || ! strlen(text))
    return "";

  safe_text = malloc ( strlen(text) * 2 );

  for ( tmp = safe_text ; * text; text ++, tmp++ )
    {
      switch ( * text )
	{
	case '&':
	case '\\':
	  *tmp = '\\';
	  *tmp++;

	default:
	  *tmp = *text;
	  break;
	}
    }
  *tmp = 0;

  return safe_text;
}
