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

#include "etats.h"
#include "etats_latex.h"

int lastline = 0;
int lastcol = 0;
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
void latex_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			  enum alignement align, struct structure_operation * ope )
{
  gchar * safe_text;
  int pad, realsize, realcolumns;

  if ( !text )
    text = "";

/*   fprintf ( out, "%% %d,%d -> %d,%d : %s\n", x, y, x2, y2, text ); */

  if ( y >= lastline )
    {
      lastcol = 0;
      lastline = y2;
      if ( ! last_is_hsep )
	{
	  fprintf ( out, "\\\\\n" );
	}
    }

  for ( pad = lastcol ; pad < x ; pad ++ )
    fprintf ( out, "&" );

  if ( (x2 - x) > 1 )
    fprintf ( out, "\\multicolumn{%d}{l}{", (x2 - x) );

  realsize = (x2 - x);
  if ( realsize > 1 )
    {
      realsize /= 2;
      if ( x == 0 )
	realsize ++;
    }
  if ( etat_courant -> afficher_opes )
    realcolumns = (float)((nb_colonnes / 2) + 1);
  else 
    realcolumns = nb_colonnes;

  fprintf ( out, 
	    "\\fboxsep \\parskip\n"
	    "\\fboxrule 0pt\n"
	    "\\begin{boxedminipage}{%f\\textwidth}\n", 
	    (float) realsize / (float) realcolumns );

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

  if ( ((int) properties) & TEXT_BOLD )
    {
      fprintf ( out, "\\bf\n");
    }
  if ( ((int) properties) & TEXT_ITALIC )
    {
      fprintf ( out, "\\em\n");
    }
  if ( ((int) properties) & TEXT_HUGE )
    {
      fprintf ( out, "\\huge\n");
    }
  if ( ((int) properties) & TEXT_LARGE )
    {
      fprintf ( out, "\\Large\n");
    }
  if ( ((int) properties) & TEXT_SMALL )
    {
      fprintf ( out, "\\small\n");
    }

  latex_safe(text);
  fprintf ( out, "\\end{boxedminipage}", safe_text );

  if ( (x2 - x) > 1 )
    fprintf ( out, "}\n" );

  fprintf ( out, "&" );

  last_is_hsep = 0;
  lastcol = x2;
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
  lastcol = x2;
}



/**
 * FIXME: TODO
 *
 */
void latex_attach_hsep ( int x, int x2, int y, int y2)
{
  if ( ! last_is_hsep )
    fprintf ( out, "\\\\\n" );
  lastline = y2;
  
  fprintf ( out, "\\hline\n" );

  last_is_hsep = 1;
  lastcol = x2;
}



gint latex_initialise (GSList * opes_selectionnees)
{
  int i;
  gfloat colwidth;

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
	   "\\usepackage{portland}\n"
	   "\\usepackage{boxedminipage}\n"
	   "\\usepackage{longtable}\n"
	   "\\usepackage{vmargin}\n"
	   "\\setpapersize{A4}\n"
	   "\\setmarginsrb{1cm}{1cm}{1cm}{1cm}{0cm}{0cm}{0cm}{0cm}\n\n"
	   "\\begin{document}\n\n"
	   "\\landscape\n"
	   "\\tabcolsep 0pt\n"
	   "\\begin{longtable}[l]{");

    if ( etat_courant -> afficher_opes )
    {
      colwidth = 27.7 / ((float) (nb_colonnes / 2) + 1 );
      fprintf ( out, "p{%fcm}", colwidth);
      for (i = 0 ; i < nb_colonnes/2 ; i++)
	{
	  fprintf ( out, "p{%fcm}p{1pt}", colwidth );
	}
      fprintf ( out, "}" );
    }
  else 
    {
      colwidth = 27.7 / (float) nb_colonnes;
      for (i = 0 ; i < nb_colonnes ; i++)
	{
	  fprintf (out, "p{%fcm}", colwidth);
	}
      fprintf (out, "p{1pt}}\n", colwidth);
    }

  return 1;
}



gint latex_finish ()
{
  fprintf (out, "\n\\end{longtable}\n"
	   "\\end{document}\n");

  fclose (out);

  if ( system ( "latex -interaction=nonstopmode toto.tex" ) > 0 )
    dialogue ( _("LaTeX run was unable to complete, see console output for details.") );
  else 
    {
      if ( !system ( "dvips -t landscape toto.dvi -o toto.ps" ) )
	{
	  system ( "gv toto.ps &" );
	}
      else 
	dialogue ( _("dvips was unable to complete, see console output for details.") );
    }
  
  return 1;
}



void latex_safe ( gchar * text ) 
{
    gboolean start = 1;

  if ( ! text || ! strlen(text))
    return;

  for ( ; * text; text ++ )
    {
      switch ( * text )
	{
	case ' ':
	  if ( start )
	    fprintf ( out, "~" );
	  else
	    fprintf ( out, "%c", *text );
	  break;

	case '&':
	case '\\':
	  fprintf ( out, "\\" );

	default:
	  start = 0;
	  fprintf ( out, "%c", *text );
	  break;
	}
    }

}
