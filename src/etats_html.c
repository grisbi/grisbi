/*  Fichier qui s'occupe d'afficher les états via une impression html */
/*      etats_html.c */

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "etats_html.h"
#include "dialog.h"
#include "etats_support.h"
#include "gsb_data_report.h"
#include "navigation.h"
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
static void html_attach_hsep ( int x, int x2, int y, int y2);
static void html_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			  enum alignement align, gint transaction_number );
static void html_attach_vsep ( int x, int x2, int y, int y2);
static gint html_finish ();
static gint html_initialise ( GSList * opes_selectionnees, gchar * filename );
static void html_safe ( gchar * text ) ;
/*END_STATIC*/

/*START_EXTERN*/
extern gint nb_colonnes;
/*END_EXTERN*/




static FILE * html_out;
static int html_lastline;
static int html_lastcol;
static gboolean html_last_is_hsep;
static gboolean html_first_line;


struct struct_etat_affichage html_affichage = {
    html_initialise,
    html_finish,
    html_attach_hsep,
    html_attach_vsep,
    html_attach_label,
};


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
 * \param transaction_number the number of a transaction to link to (not used as html
 *            backend is not interactive)
 */
void html_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			  enum alignement align, gint transaction_number )
{
    int pad, realsize, realcolumns;
    gint current_report_number;

    current_report_number = gsb_gui_navigation_get_current_report ();


    if ( !text )
	text = "";

    if ( y >= html_lastline )
    {
	html_lastcol = 0;
	html_lastline = y2;
	if ( ! html_first_line )
	  {
	    fprintf ( html_out, "      </tr>\n\n");
	  }
	html_first_line = FALSE;
	fprintf ( html_out, "      <tr>\n");
    }

    for ( pad = html_lastcol ; pad < x ; pad ++ )
	fprintf ( html_out, "        <td></td>\n" );

    fprintf ( html_out, "        <td" );

    if ( (x2 - x) > 1 )
	fprintf ( html_out, " colspan=\"%d\"", (x2 - x) );

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

    switch ( align )
    {
	case ALIGN_LEFT:
	    fprintf ( html_out, " align=\"left\"" );
	    break;

	case ALIGN_RIGHT:
	    fprintf ( html_out, " align=\"right\"" );
	    break;

	case ALIGN_CENTER:
	    fprintf ( html_out, " align=\"center\"" );
	    break;
    }

    fprintf ( html_out, ">&nbsp;" );

    if ( ((int) properties) & TEXT_BOLD )
    {
	fprintf ( html_out, "<b>");
    }
    if ( ((int) properties) & TEXT_ITALIC )
    {
	fprintf ( html_out, "<em>");
    }
    if ( ((int) properties) & TEXT_HUGE )
    {
	fprintf ( html_out, "<font size=\"+5\">");
    }
    if ( ((int) properties) & TEXT_LARGE )
    {
	fprintf ( html_out, "<font size=\"+2\">");
    }
    if ( ((int) properties) & TEXT_SMALL )
    {
	fprintf ( html_out, "<font size=\"-2\">");
    }

    html_safe(text);

    if ( ((int) properties) & TEXT_SMALL )
    {
	fprintf ( html_out, "</font>");
    }
    if ( ((int) properties) & TEXT_LARGE )
    {
	fprintf ( html_out, "</font>");
    }
    if ( ((int) properties) & TEXT_HUGE )
    {
	fprintf ( html_out, "</font>");
    }
    if ( ((int) properties) & TEXT_ITALIC )
    {
	fprintf ( html_out, "</em>");
    }
    if ( ((int) properties) & TEXT_BOLD )
    {
	fprintf ( html_out, "</b>");
    }

    fprintf ( html_out, "        </td>\n" );

    html_last_is_hsep = 0;
    html_lastcol = x2;
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
void html_attach_vsep ( int x, int x2, int y, int y2)
{ 
  int pad;

  if ( y >= html_lastline )
    {
      if ( ! html_first_line )
	{
	  fprintf ( html_out, "      </tr>\n\n");
	}
      fprintf ( html_out, "      <tr>\n");
      html_lastline = y2;
    }

  for ( pad = html_lastcol ; pad < x ; pad ++ )
    fprintf ( html_out, "        <td></td>" );

  fprintf ( html_out, "        <td width=\"1\" bgcolor=\"black\"></td>\n" );

  html_last_is_hsep = 0;
  html_lastcol = x2;
  html_first_line = FALSE;
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
void html_attach_hsep ( int x, int x2, int y, int y2)
{
  if ( ! html_first_line )
    {
      fprintf ( html_out, "      </tr>\n\n");
    }

  fprintf ( html_out, 
	    "      <tr>\n"
	    "        <td colspan=\"%d\">\n"
	    "          <hr/>\n" 
	    "        </td>\n",
	    nb_colonnes );

  html_last_is_hsep = 1;
  html_lastline = y2;
  html_lastcol = x2;
  html_first_line = FALSE;
}



/**
 * Initialization function for the html backend.
 *
 * \return TRUE on succes, FALSE otherwise.
 */
gint html_initialise ( GSList * opes_selectionnees, gchar * filename )
{
    g_return_val_if_fail ( filename, FALSE );

    html_lastline = -1;
    html_lastcol = 0;
    html_last_is_hsep = FALSE;
    html_first_line = TRUE;

    html_out = utf8_fopen ( filename, "w" );
    if ( ! html_out )
    {
      dialogue_error_hint ( _("Make sure file exists and is writable."),
			   g_strdup_printf (_("Cannot open file '%s' for writing"), filename));
      return FALSE;
    }

    fprintf (html_out, 
	     "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
	     "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\"\n"
	     "  \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n\n"
	     "<html>\n"
	     "  <head>\n"
	     "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\n"
	     "    <title>");

    if (!gsb_gui_navigation_get_current_report ())
	return FALSE;

    html_safe (etats_titre(gsb_gui_navigation_get_current_report ()));
    fprintf (html_out, 
	     "</title>\n"
	     "  </head>\n\n"
	     "  <body>\n"
	     "    <table cellspacing=\"0\" cellpadding=\"0\" border=\"0\">\n\n");

    return TRUE;
}



/**
 * Destructor function for the html backend.
 *
 * \return TRUE on success, FALSE otherwise.
 */
gint html_finish ()
{
    fprintf (html_out, 
	     "      </tr>\n\n"
	     "    </table>\n"
	     "  </body>\n"
	     "</html>\n");
    fclose (html_out);

    return 1;
}



/** 
 * Print a html safe string into the html_out file descriptor.  All
 * chars that cannot be printed via html are converted to their html
 * equivalent
 *
 * \param text Text to print.
 */
void html_safe ( gchar * text ) 
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
		    fprintf ( html_out, "&nbsp;" );
		else
		    fprintf ( html_out, "%c", *text );
		break;

	    case '&':
		fprintf ( html_out, "&amp;" );
		break;

	    case '<':
		fprintf ( html_out, "&lt;" );
		break;

	    case '>':
		fprintf ( html_out, "&gt;" );
		break;

	    default:
		start = 0;
		fprintf ( html_out, "%c", *text );
		break;
	}
    }

}
