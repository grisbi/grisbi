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

#include "include.h"
#include "structures.h"
#include "variables-extern.c"
#include "etats_html.h"

#include "dialog.h"
#include "etats.h"
#include "etats_support.h"
#include "utils_files.h"
#include "utils_file_selection.h"


FILE * html_out;
gchar * html_tempname;
int html_lastline;
int html_lastcol;
gboolean html_last_is_hsep;
gboolean html_first_line;


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
 * \param ope a pointer to a transaction to link to (not used as html
 *            backend is not interactive)
 */
void html_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			  enum alignement align, struct structure_operation * ope )
{
    int pad, realsize, realcolumns;

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
	if ( etat_courant -> afficher_opes )
	{
	    realsize /= 2;
	    if ( x == 0 )
		realsize ++;
	}
    }
    if ( etat_courant -> afficher_opes )
	realcolumns = (float)((nb_colonnes / 2) + 1);
    else 
	realcolumns = nb_colonnes;

    switch ( align )
    {
	case LEFT:
	    fprintf ( html_out, " align=\"left\"" );
	    break;

	case RIGHT:
	    fprintf ( html_out, " align=\"right\"" );
	    break;

	case CENTER:
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
gint html_initialise (GSList * opes_selectionnees)
{
    GtkWidget * file_selector;
    gchar * filename;
    gint resultat;

    html_lastline = -1;
    html_lastcol = 0;
    html_last_is_hsep = FALSE;
    html_first_line = TRUE;

    file_selector = gtk_file_selection_new ( _("Export report to HTML file."));
    file_selection_set_filename ( GTK_FILE_SELECTION ( file_selector ),
				  dernier_chemin_de_travail );

    file_selection_set_entry ( GTK_FILE_SELECTION ( file_selector ), 
			       safe_file_name ( g_strconcat (etats_titre(), ".html", NULL)));

    do 
    {
	resultat = gtk_dialog_run ( GTK_DIALOG (file_selector) );
	if ( resultat == GTK_RESPONSE_OK )
	{
	    FILE * test;
	    gchar * filename;

	    filename = file_selection_get_filename ( GTK_FILE_SELECTION ( file_selector ));

	    test = utf8_fopen ( filename, "r" );
	    if ( test )
	    {
		fclose ( test );
		if ( question_yes_no_hint ( g_strdup_printf ( _("File %s already exists."), 
							      filename ),
					    _("Do you want to overwrite it?  There is no undo for this.") ) )
		{
		    break;
		}
	    }
	    else
	    {
		break;
	    }
	}
	else
	{
	    break;
	}
    }
    while ( 1 );

    switch ( resultat )
      {
      case GTK_RESPONSE_OK :
	filename = file_selection_get_filename ( GTK_FILE_SELECTION ( file_selector ));
	break;
	
      default:
	gtk_widget_destroy ( file_selector );
	return FALSE;
      }
    gtk_widget_destroy ( file_selector );
    
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
    /* FIXME: be sure this works if displayed report is not the current one. */
    html_safe (etats_titre());
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
