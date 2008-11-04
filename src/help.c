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
#include "./utils.h"
#include "./utils_str.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void launch_url (GtkAboutDialog *about, const gchar * link, gpointer data);
/*END_STATIC*/


/*START_EXTERN*/
extern gchar *chemin_logo ;
/*END_EXTERN*/




/**
 * Handler used to pop up a web browser when user clicked on a link in
 * the GtkAboutDialog.
 * 
 * \param about		Dialog that triggered the event.
 * \param link		URL to display.
 * \param data		Not used.
 */
void launch_url (GtkAboutDialog *about, const gchar * link, gpointer data)
{
    lance_navigateur_web ( link );
}



/**
 * Create and run an About dialog.
 *
 * \param bouton	Widget that triggered this handler (not used).
 * \param data		Not used.
 */
void a_propos ( GtkWidget *bouton,
		gint data )
{
    GdkPixbuf * logo;

#define CSUFFIX "\n"

    const gchar *auteurs[] = {
_("Programming"),
"Benjamin Drieu (bdrieu[at]april.org)",
"Cedric Auger (cedric[at]grisbi.org)",
"Francois Terrot (grisbi[at]terrot.net)",
"Pierre Bavia ([at]terrot.net)",
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

  const gchar *documenters [] = {
"Alain Portal (aportal[at]univ-monpt2.fr)",
"Daniel Cartron (doc[at]grisbi.org)",
NULL};

  const gchar *artists [] = {
"Andre Pascual (andre[at]linuxgraphic.org)",
NULL};

  const gchar *license = "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n\n"
	"This product includes software developed by the OpenSSL Project for use in the OpenSSL Toolkit. (http://www.openssl.org/)";

#if ! GTK_CHECK_VERSION(2,10,0)
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

    GtkWidget * about;

    /* Logo */
    if ( !chemin_logo || !strlen ( chemin_logo ))
	chemin_logo = my_strdup ( LOGO_PATH );
    logo =  gdk_pixbuf_new_from_file ( chemin_logo, NULL );

    about = gtk_about_dialog_new ( );
    gtk_about_dialog_set_url_hook (launch_url, NULL, NULL);
    gtk_about_dialog_set_name ( GTK_ABOUT_DIALOG (about), "Grisbi" );
    gtk_about_dialog_set_logo ( GTK_ABOUT_DIALOG (about), logo );
    gtk_about_dialog_set_comments ( GTK_ABOUT_DIALOG (about), comments );
    gtk_about_dialog_set_website ( GTK_ABOUT_DIALOG (about), "http://www.grisbi.org/");
    gtk_about_dialog_set_license ( GTK_ABOUT_DIALOG (about), license );
    gtk_about_dialog_set_wrap_license ( GTK_ABOUT_DIALOG (about), TRUE );
    gtk_about_dialog_set_version ( GTK_ABOUT_DIALOG (about), VERSION );
    gtk_about_dialog_set_artists ( GTK_ABOUT_DIALOG (about), artists );
    gtk_about_dialog_set_documenters ( GTK_ABOUT_DIALOG (about), documenters );
    gtk_about_dialog_set_authors ( GTK_ABOUT_DIALOG (about), auteurs );
    gtk_about_dialog_set_translator_credits ( GTK_ABOUT_DIALOG (about), translators );
                                  
    gtk_dialog_run ( GTK_DIALOG (about)) ;
    
    gtk_widget_destroy (about);
                                  
    if (logo)
	g_object_unref (logo);

    return;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
