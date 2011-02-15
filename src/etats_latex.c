/*  Fichier qui s'occupe d'afficher les états via une impression latex */
/*      etats_latex.c */

/*     Copyright (C)	2004-2009 Benjamin Drieu (bdrieu@april.org) */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "etats_latex.h"
#include "dialog.h"
#include "gsb_data_report.h"
#include "navigation.h"
#include "print_config.h"
#include "utils_files.h"
#include "structures.h"
#include "etats_config.h"
#include "etats_affiche.h"
/*END_INCLUDE*/

/* FOLLOWING LINES REVERTS BACK TO THE ORIGINAL FPRINTF (LIBINTL_FPRINTF IS BUGGY) */
#ifdef _WIN32
	#undef fprintf
#endif

/*START_STATIC*/
static void latex_attach_hsep ( int x, int x2, int y, int y2);
static void latex_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2,
			  enum alignement align, gint transaction_number );
static void latex_attach_vsep ( int x, int x2, int y, int y2);
static gint latex_finish ();
static gint latex_initialise (GSList * opes_selectionnees, gchar * filename );
static void latex_safe ( gchar * text );
/*END_STATIC*/




static gint lastline = 0;
static gint lastcol = 0;
static gint last_is_hsep = 0;
static FILE *file_out;
static gchar * tempname;


struct struct_etat_affichage latex_affichage = {
    latex_initialise,
    latex_finish,
    latex_attach_hsep,
    latex_attach_vsep,
    latex_attach_label,
};

/*START_EXTERN*/
extern gint nb_colonnes;
/*END_EXTERN*/


/**
 * Backend function that is responsible for printing a label at a
 * specific place of the report, using text attibutes.
 *
 * \param text Text to print
 * \param properties bit field for text properties.  Can be:
 *        - TEXT_BOLD: text will be printed in bold
 *        - TEXT_ITALIC: text will be printed in intalic
 *        - TEXT_HUGE: text will be have a huge face
 *        - TEXT_LARGE: text will be have a large face
 *        - TEXT_SMALL: text will be have a small face
 * \param x left position of the label
 * \param y top position of the label
 * \param x2 right position of the label
 * \param y2 bottom position of the label
 * \param alignment how the text will be aligned in the cell
 * \param transaction_number a number of transaction to link to (not used as latex
 *            backend is not interactive)
 */
void latex_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2,
			  enum alignement align, gint transaction_number )
{
    int pad, realsize, realcolumns;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( !text )
	text = "";

    if ( y >= lastline )
    {
	lastcol = 0;
	lastline = y2;
	if ( ! last_is_hsep )
	{
	    fprintf ( file_out, "\\\\\n" );
	}
    }

    for ( pad = lastcol ; pad < x ; pad ++ )
	fprintf ( file_out, "&" );

    if ( (x2 - x) > 1 )
	fprintf ( file_out, "\\multicolumn{%d}{l}{", (x2 - x) );

    realsize = (x2 - x);
    if ( realsize > 1 )
    {
	if ( gsb_data_report_get_show_report_transactions (current_report_number))
	{
	    realsize /= 2;
	    if ( x == 0 )
		realsize ++;
	}
    }
    if ( gsb_data_report_get_show_report_transactions (current_report_number))
	realcolumns = (float)((nb_colonnes / 2) + 1);
    else
	realcolumns = nb_colonnes;

    fprintf ( file_out,
	      "\\begin{boxedminipage}{%f\\text%s}\n",
	      (float) realsize / (float) realcolumns,
	      ( etat.print_config.orientation == LANDSCAPE ? "height" : "width") );

    switch ( align )
    {
	case ALIGN_LEFT:
	    fprintf ( file_out, "\\raggedright\n" );
	    break;

	case ALIGN_RIGHT:
	    fprintf ( file_out, "\\raggedleft\n" );
	    break;

	case ALIGN_CENTER:
	    break;
    }

    if ( ((int) properties) & TEXT_BOLD )
    {
	fprintf ( file_out, "\\bf\n");
    }
    if ( ((int) properties) & TEXT_ITALIC )
    {
	fprintf ( file_out, "\\em\n");
    }
    if ( ((int) properties) & TEXT_HUGE )
    {
	fprintf ( file_out, "\\huge\n");
    }
    if ( ((int) properties) & TEXT_LARGE )
    {
	fprintf ( file_out, "\\Large\n");
    }
    if ( ((int) properties) & TEXT_SMALL )
    {
	fprintf ( file_out, "\\small\n");
    }

    latex_safe(text);
    fprintf ( file_out, "\\end{boxedminipage}" );

    if ( (x2 - x) > 1 )
	fprintf ( file_out, "}\n" );

    fprintf ( file_out, "&" );

    last_is_hsep = 0;
    lastcol = x2;
}



/**
 * Backend function that is responsible for printing a vertical
 * separator (i.e. a vertical line).
 *
 * \param x left position of the separator
 * \param y top position of the separator
 * \param x2 right position of the separator
 * \param y2 bottom position of the separator
 */
void latex_attach_vsep ( int x, int x2, int y, int y2)
{
    int pad;

    if ( y >= lastline )
    {
	if ( ! last_is_hsep )
	    fprintf ( file_out, "\\\\\n" );
	lastline = y2;
    }

    for ( pad = lastcol ; pad < x ; pad ++ )
	fprintf ( file_out, "&" );

    fprintf ( file_out, "{\\vrule}&" );

    last_is_hsep = 0;
    lastcol = x2;
}



/**
 * Backend function that is responsible for printing an horizontal
 * separator (i.e. an horizontal line).
 *
 * \param x left position of the separator
 * \param y top position of the separator
 * \param x2 right position of the separator
 * \param y2 bottom position of the separator
 */
void latex_attach_hsep ( int x, int x2, int y, int y2)
{
    if ( ! last_is_hsep )
	fprintf ( file_out, "\\\\\n" );
    lastline = y2;

    fprintf ( file_out, "\\hline\n" );

    last_is_hsep = 1;
    lastcol = x2;
}



/**
 * Initialization function for the latex backend.
 *
 * \return TRUE on succes, FALSE otherwise.
 */
gint latex_initialise (GSList * opes_selectionnees, gchar * filename )
{
    gfloat colwidth, real_width;
    gint i, current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();

    if ( etat.print_config.printer || etat.print_config.filetype == POSTSCRIPT_FILE )
    {
	tempname = g_strdup_printf ( "gsbpt%d", g_random_int_range (0,99999) );
	filename = g_strdup_printf ( "%s.tex", tempname );
    }
    else
    {
	filename = etat.print_config.printer_filename;
    }

    if ( ! filename )
    {
	filename = "";
    }

    g_unlink ( filename );
    file_out = utf8_fopen ( filename, "w" );
    if ( ! file_out )
    {
	dialogue_error ( g_strdup_printf (_("Cannot open file '%s': %s."), filename, g_strerror ( errno ) ) );
	return FALSE;
    }

    fprintf (file_out,
	     "\\documentclass{article}\n\n"
	     "\\special{! TeXDict begin /landplus90{true}store end }\n"
	     "\\usepackage{a4}\n"
	     "\\usepackage{eurosym}\n"
	     "\\usepackage{boxedminipage}\n"
	     "\\usepackage{longtable}\n"
	     "\\usepackage{vmargin}\n"
	     "\\usepackage[T1]{fontenc}\n"
	     "\\usepackage{ucs}\n"
	     "\\usepackage[utf8x]{inputenc}\n" );

    if ( etat.print_config.orientation == LANDSCAPE )
    {
	fprintf (file_out, "\\usepackage{portland}\n");
    }

    fprintf (file_out,
	     "\\setpapersize{%s}\n"
	     "\\setmarginsrb{1cm}{1cm}{1cm}{1cm}{0cm}{0cm}{0cm}{0cm}\n\n"
	     "\\begin{document}\n\n", etat.print_config.paper_config.name);

    if ( etat.print_config.orientation == LANDSCAPE )
      {
	fprintf (file_out, "\\landscape\n\n");
      }

    fprintf (file_out,
	     "\\fboxsep \\parskip\n"
	     "\\fboxrule 0pt\n"
	     "\\tabcolsep 0pt\n"
	     "\\begin{longtable}[l]{" );

    if ( etat.print_config.orientation == LANDSCAPE )
    {
	real_width = ((etat.print_config.paper_config.height-20)/10);
    }
    else
    {
	real_width = ((etat.print_config.paper_config.width-20)/10);
    }

    if ( gsb_data_report_get_show_report_transactions (current_report_number))
    {
	colwidth = real_width / ((float) (nb_colonnes / 2) + 1 );
	fprintf ( file_out, "p{%fcm}", colwidth);
	for (i = 0 ; i < nb_colonnes/2 ; i++)
	{
	    fprintf ( file_out, "p{%fcm}p{1pt}", colwidth );
	}
	fprintf ( file_out, "}" );
    }
    else
    {
	colwidth = real_width / (float) nb_colonnes;
	for (i = 0 ; i < nb_colonnes ; i++)
	{
	    fprintf (file_out, "p{%fcm}", colwidth);
	}
	fprintf (file_out, "p{1pt}}\n");
    }

    return TRUE;
}



/**
 * Destructor function for the latex backend.
 *
 * \return TRUE on success, FALSE otherwise.
 */
gint latex_finish ()
{
    gchar * command;

    fprintf (file_out, "\n"
	     "\\end{longtable}\n"
	     "\\end{document}\n");
    fclose (file_out);

    if ( etat.print_config.printer || etat.print_config.filetype == POSTSCRIPT_FILE )
    {
        command = g_strdup_printf ( "%s -interaction=nonstopmode \"%s.tex\"",
                        conf.latex_command, tempname );
        if ( system ( command ) > 0 )
            dialogue_error_hint ( _("See console output for details.  Be sure you have installed LaTeX properly with unicode support."),
                      _("LaTeX run was unable to complete.") );
        else
        {
            command = g_strdup_printf ( "%s %s \"%s.dvi\" -o \"%s\"",  conf.dvips_command,
                        ( etat.print_config.orientation == LANDSCAPE ? "-t landscape" : ""),
                        tempname,
                        (etat.print_config.printer ?
                         (g_strconcat ( tempname, ".ps", NULL )) :
                         etat.print_config.printer_filename) );
            g_unlink ( g_strdup_printf ("%s.tex", tempname) );
            g_unlink ( g_strdup_printf ("%s.aux", tempname) );
            g_unlink ( g_strdup_printf ("%s.log", tempname) );
            if ( !system ( command ) )
            {
            if ( etat.print_config.printer )
            {
                command = g_strdup_printf ( "%s %s.ps", etat.print_config.printer_name,
                            tempname );
                if ( system ( command ) )
                {
                dialogue_error ( _("Cannot send job to printer") );
                }
            }
            }
            else
            {
            dialogue_error ( _("dvips was unable to complete, see console output for details.") );
            }
            g_unlink ( g_strdup_printf ("%s.dvi", tempname) );
	}

	if ( etat.print_config.printer )
	{
	    g_unlink ( g_strdup_printf ("%s.ps", tempname) );
	}

	g_free ( tempname );
    }

    return 1;
}



/**
 * Print a latex safe string into the file_out file descriptor.  All chars
 * that cannot be printed via latex are converted to their latex
 * equivalent (i.e. backslashes are escaped).
 *
 * \param text Text to print.
 */
void latex_safe ( gchar * text )
{
    gboolean start = 1;

    if ( ! text || ! strlen(text))
	return;

    for ( ; * text; text ++ )
    {
	guchar c = * text;
	switch ( c )
	{

	    case ' ':
		if ( start )
		    fprintf ( file_out, "~" );
		else
		    fprintf ( file_out, "%c", *text );
		break;

	    case '_':
	    case '&':
	    case '%':
	    case '#':
	    case '\\':
	    case '$':
		fprintf ( file_out, "\\" );

	    default:
		start = 0;
		fprintf ( file_out, "%c", *text );
		break;
	}
    }
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
