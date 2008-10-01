/* Fichier help.c */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2008 Benjamin Drieu (bdrieu@april.org) */
/* 			http://www.grisbi.org */

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


/*START_INCLUDE*/
#include "help.h"
#include "./gsb_plugins.h"
#include "./utils_str.h"
#include "./structures.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern gchar *chemin_logo ;
extern GtkWidget *window ;
/*END_EXTERN*/




/* **************************************************************************************************************************** */
void a_propos ( GtkWidget *bouton,
		gint data )
{
    GdkPixbuf * logo;

#define CSUFFIX "\n"

    gchar *auteurs[] = {
_("Programming"),
"Benjamin Drieu (bdrieu[at]april.org)",
"Cedric Auger (cedric[at]grisbi.org)",
"Francois Terrot (grisbi[at]terrot.net)",
"",

_("Packaging"),
"Alain Pichon (aph[at]grisbi.org)",
"Baluchiterium  (baluchiterium[at]users.sf.net)",
"Francois Terrot (grisbi[at]terrot.net)",
"Gerald Niel (gerald.niel[at]grisbi.org)",
"PMF (ugly.duck[at]gmx.de)",
"Pascal Bleser (guru[at]linuxbe.org)",
"Sylvain Glaize (mokona[at]puupuu.org)",
"Thierry Thomas (thierry[at]pompo.net)",
"Vincent Marqueton (vincent[at]marqueton.com)",
"\n",

_("Other"),
"Axel Rousseau (axel584[at]axel584.org) ",
"Dominique Parisot (parisot[at]villey-le-sec.com) ",
"Gerald Niel (gerald.niel[at]grisbi.org) ",
NULL };	

  gchar * translators = g_strconcat(
"Alain Portal (dionysos[at]grisbi.org): ", _("English"), CSUFFIX,
"Benjamin Drieu (bdrieu[at]april.org): ", _("English"), CSUFFIX,
"Carlos M. Cámara Mora (carcam_moceu[at]yahoo.es): ", _("Spanish"), CSUFFIX,
"Daniel Cartron (cartron[at]grisbi.org): ", _("English"), CSUFFIX,
"Edwin Huijsing (e.huijsing[at]fiberworld.nl): ", _("Dutch"), CSUFFIX,
"Fabio Erculiani (fabio.erculiani[at]tiscali.it): ", _("Italian"), CSUFFIX,
"Flavio Henrique Somensi (flavio[at]opens.com.br): ", _("Brazilian Portuguese"), CSUFFIX,
"Giorgio Mandolfo (giorgio[at]pollycoke.org): ", _("Italian"), CSUFFIX,
"Martin Stromberger (Fabiolla[at]aon.at): ", _("German"), CSUFFIX,
"Ryszard Jeziorski (rjeziorski[at]eagle): ", _("Polish"), CSUFFIX,
NULL);

  gchar *documenters [] = {
"Alain Portal (aportal[at]univ-monpt2.fr)",
"Daniel Cartron (doc[at]grisbi.org)",
NULL};

  gchar *artists [] = {
"Andre Pascual (andre[at]linuxgraphic.org)",
NULL};

  const gchar *liens[] = { 
	COLON(_("Website")),
	"http://www.grisbi.org/",
	"http://www.grisbi.org/",
	_("Development list (discuss, participate, criticize, ...) : "),
	"http://lists.sourceforge.net/lists/listinfo/grisbi-devel",
	_("subscribe"),
	COLON(_("Bug report")),
	"http://www.grisbi.org/bugtracking/",
	"http://www.grisbi.org/bugtracking/",
	_("Information list (new releases announces, ...) : "),
	"http://lists.sourceforge.net/lists/listinfo/grisbi-info",
	_("subscribe"),
	COLON(_("Grisbi documentation")),
	"http://www.grisbi.org/manuel.html",
	"http://www.grisbi.org/manuel.html",
	NULL };

    const gchar *license = "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n\n"
	"This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit. (http://www.openssl.org/)";

#if ! GTK_CHECK_VERSION(2,20,0)
    // Warn about obsolete dependencies 
    gchar * warn_print = _("\n\nThis version of Grisbi does not support print feature.\nVersion of GTK+ it was built with is obsolete.");
#else
    gchar * warn_print = NULL;
#endif

    /* Plugins list */
    gchar* comments = g_strconcat ( _("Personal finance manager for everyone\n"),
				    gsb_plugin_get_list(),
				    warn_print,
				    NULL );

    /* Logo */
    if ( !chemin_logo || !strlen ( chemin_logo ))
	chemin_logo = my_strdup ( LOGO_PATH );
    logo =  gdk_pixbuf_new_from_file ( chemin_logo, NULL );

    gtk_show_about_dialog (GTK_WINDOW (window), 
			   "logo", logo,
                           "program-name", "Grisbi",
			   "comments", comments,
			   "artists", artists,
			   "authors", auteurs,
			   "documenters", documenters,
			   "translator-credits", translators,
			   "version", VERSION,
                           "license", license,
			   "wrap-license", TRUE,
			   "website", "http://www.grisbi.org/",
			   NULL);

    if (logo)
	g_object_unref (logo);

    return;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
