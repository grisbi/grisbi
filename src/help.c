/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)    2000-2003 Cédric Auger (cedric@grisbi.org)            */
/*          2008 Benjamin Drieu (bdrieu@april.org)                            */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          https://www.grisbi.org/                                           */
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
/*  along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                            */
/* ************************************************************************** */


#include "config.h"

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "help.h"
#include "grisbi_app.h"
#include "gsb_select_icon.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/


/**
 * Handler used to lauch a mailer or browser when user clicked on a link in
 * the GtkAboutDialog.
 *
 * \param label     link that triggered the event.
 * \param uri       URL to display.
 * \param data      Not used.
 */
static gboolean url_clicked ( GtkAboutDialog *label,
                        gchar *uri,
                        gpointer user_data )
{
    if ( g_str_has_prefix ( uri, "mailto:" ) )
    {
        gchar *str;

        str = gsb_string_remplace_string ( uri, "%40", "@" );
        lance_mailer ( str );
        g_free ( str );
    }
    else
        lance_navigateur_web ( uri );

    return TRUE;
}


gchar * extra_support ( void )
{
    gchar *ret;

    ret = g_strconcat (
        _("GnuCash support: "),
#ifdef HAVE_XML2
        _("yes"), "\n",
#else
        _("no"), "\n",
#endif
        _("OFX support: "),
#ifdef HAVE_OFX
        _("yes"), "\n",
#else
        _("no"), "\n",
#endif
        _("OpenSSL support: "),
#ifdef HAVE_SSL
        _("yes"), "\n",
#else
        _("no"), "\n",
#endif
        NULL);

    return ret;
}


/**
 * Create and run an About dialog.
 *
 * \param bouton	Widget that triggered this handler (not used).
 * \param data		Not used.
 */
void a_propos ( GtkWidget *bouton, gint data )
{
    GtkWidget *about;
    GdkPixbuf *logo;
	gboolean logo_is_local = FALSE;

#define CSUFFIX "\n"

    const gchar *auteurs[] = {
_("Programming"),
"Benjamin Drieu <bdrieu@april.org>",
"Cedric Auger <cedric@grisbi.org>",
"Francois Terrot <grisbi@terrot.net>",
"Pierre Biava <pierre@pierre.biava.name>",
"Mickaël Remars <grisbi@remars.com>",
"William Ollivier <guneeyoufix@gmail.com>",
"Rémi Cardona <remi@gentoo.org>",
"Ludovic Rousseau <ludovic.rousseau@free.fr>"
"\n",

_("Packaging"),
"Alain Pichon <aph@grisbi.org>",
"Baluchiterium  <baluchiterium@users.sf.net>",
"Francois Terrot <grisbi@terrot.net>",
"Gerald Niel <gerald.niel@grisbi.org>",
"Gilles Morel <g.morel@gmail.com>",
"PMF <ugly.duck@gmx.de>",
"Pascal Bleser <guru@linuxbe.org>",
"Sylvain Glaize <mokona@puupuu.org>",
"Thierry Thomas <thierry@pompo.net>",
"Vincent Marqueton <vincent@marqueton.com>",
"William Ollivier <guneeyoufix@gmail.com>",
"Frederic Trouche <fred@linxutribe.org>",
"\n",

_("Other"),
"Axel Rousseau <axel584@axel584.org>",
"Dominique Parisot <parisot@villey-le-sec.com>",
"Gerald Niel <gerald.niel@grisbi.org>", CSUFFIX,
NULL };

  gchar * translators = g_strconcat(
"Alain Portal <dionysos@grisbi.org>: ", _("English"), CSUFFIX,
"Benjamin Drieu <bdrieu@april.org>: ", _("English"), CSUFFIX,
"Carlos M. Cámara Mora <carcam_moceu@yahoo.es>: ", _("Spanish"), CSUFFIX,
"Daniel Cartron <cartron@grisbi.org>: ", _("English"), CSUFFIX,
"Edwin Huijsing <e.huijsing@fiberworld.nl>: ", _("Dutch"), CSUFFIX,
"Fabio Erculiani <fabio.erculiani@tiscali.it>: ", _("Italian"), CSUFFIX,
"Flavio Henrique Somensi <flavio@opens.com.br>: ", _("Brazilian Portuguese"), CSUFFIX,
"Giorgio Mandolfo <giorgio@pollycoke.org>: ", _("Italian"), CSUFFIX,
"Martin Stromberger <ma.stromberger@gmx.at>: ", _("German"), CSUFFIX,
"Ryszard Jeziorski <rjeziorski@eagleW>: ", _("Polish"), CSUFFIX,
"Achilleas Kaskamanidis <alterna55@users.sourceforge.net>: ", _("Greek"), CSUFFIX,
NULL);

  const gchar *documenters [] = {
"Alain Portal <aportal@univ-monpt2.fr>",
"Benjamin Drieu <bdrieu@april.org>",
"Daniel Cartron <doc@grisbi.org>",
"Jean-Luc Duflot <jl.duflot@laposte.net>",
"Loic Breilloux <loic.breilloux@libertysurf.fr>", CSUFFIX,
NULL};

  const gchar *artists [] = {
"Andre Pascual <andre@linuxgraphic.org>",
"Jakub 'jimmac' Steiner <jimmac@ximian.com>",
"Jean-Baptiste Renard <renardjb@free.fr>",
"Tuomas 'tigert' Kuosmanen <tigert@gimp.org>",
NULL};

  const gchar *license = "This program is free software; you can redistribute "
                        "it and/or modify it under the terms of the GNU General "
                        "Public License as published by the Free Software Foundation; "
                        "either version 2 of the License, or (at your option) any later "
                        "version.\n\nThis program is distributed in the hope that it "
                        "will be useful, but WITHOUT ANY WARRANTY; without even the "
                        "implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR "
                        "PURPOSE.  See the GNU General Public License for more details.\n\n"
                        "You should have received a copy of the GNU General Public License "
                        "along with this program; if not, see <https://www.gnu.org/licenses/>.\n\n"
                        "This product includes software developed by the OpenSSL Project "
                        "for use in the OpenSSL Toolkit.\n(https://www.openssl.org/)";

    /* Others info */
    gchar *comments;
    gchar *extra;
    gchar *version_to_string;

    extra = extra_support ( );
    version_to_string = get_gtk_run_version ( );

	comments = g_strconcat ( _("Personal finance manager for everyone\n"),
					extra,
					_("GTK+ version: "),
					version_to_string,
#ifdef DEBUG
					"\n",
					_("Grisbi git hash: "),
					GIT_HASH,
#endif
					NULL );

    /* Logo */
    logo = gsb_select_icon_get_logo_pixbuf ( );
    if (logo == NULL )
    {
        logo =  gsb_select_icon_get_default_logo_pixbuf ();
		logo_is_local = TRUE;
    }

    about = gtk_about_dialog_new ( );
    gtk_about_dialog_set_program_name ( GTK_ABOUT_DIALOG (about), "Grisbi" );
    gtk_about_dialog_set_logo ( GTK_ABOUT_DIALOG (about), logo );
    gtk_about_dialog_set_comments ( GTK_ABOUT_DIALOG (about), comments );
    gtk_about_dialog_set_website ( GTK_ABOUT_DIALOG (about), "https://www.grisbi.org/");
    gtk_about_dialog_set_license ( GTK_ABOUT_DIALOG (about), license );
    gtk_about_dialog_set_wrap_license ( GTK_ABOUT_DIALOG (about), TRUE );
    gtk_about_dialog_set_version ( GTK_ABOUT_DIALOG (about), VERSION );
    gtk_about_dialog_set_artists ( GTK_ABOUT_DIALOG (about), artists );
    gtk_about_dialog_set_documenters ( GTK_ABOUT_DIALOG (about), documenters );
    gtk_about_dialog_set_authors ( GTK_ABOUT_DIALOG (about), auteurs );
    gtk_about_dialog_set_translator_credits ( GTK_ABOUT_DIALOG (about), translators );

    gtk_window_set_position ( GTK_WINDOW ( about ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( about ), TRUE );
    gtk_window_set_modal ( GTK_WINDOW ( about ), TRUE );
    gtk_window_set_transient_for ( GTK_WINDOW ( about ), GTK_WINDOW ( grisbi_app_get_active_window (NULL) ) );

    g_free ( extra );
    g_free ( version_to_string );
    g_free ( comments );
    g_signal_connect ( G_OBJECT ( about ),
                        "activate-link",
                        G_CALLBACK ( url_clicked ),
                        NULL );

    gtk_dialog_run ( GTK_DIALOG (about)) ;

    gtk_widget_destroy (about);

	if (logo_is_local)
		g_object_unref (logo);

    return;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
