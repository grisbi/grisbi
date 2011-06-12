/* ************************************************************************** */
/*                                                                            */
/*     copyright (c)    2000-2003 Cédric Auger (cedric@grisbi.org)            */
/*          2008 Benjamin Drieu (bdrieu@april.org)                            */
/*          2008-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
/*          http://www.grisbi.org                                             */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "help.h"
#include "utils.h"
#include "utils_dates.h"
#include "gsb_plugins.h"
#include "gsb_select_icon.h"
#include "utils_str.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void launch_url (GtkAboutDialog *about, const gchar * link, gpointer data);
/*END_STATIC*/


/*START_EXTERN*/
extern GtkWidget *window;
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
 * Handler used to pop up a mailer when user clicked on a link in
 * the GtkAboutDialog.
 *
 * \param about		Dialog that triggered the event.
 * \param link		URL to display.
 * \param data		Not used.
 */
void launch_mailto (GtkAboutDialog *about, const gchar * link, gpointer data)
{
    lance_mailer ( link );
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
    gchar *chemin_logo ;

#define CSUFFIX "\n"

    const gchar *auteurs[] = {
_("Programming"),
"Benjamin Drieu (<bdrieu@april.org>)",
"Cedric Auger (<cedric@grisbi.org>)",
"Francois Terrot (<grisbi@terrot.net>)",
"Pierre Biava (<pierre.biava@nerim.net>)",
"Mickaël Remars (<grisbi@remars.com>)",
"William Ollivier (<guneeyoufix@gmail.com>)"
"\n",

_("Packaging"),
"Alain Pichon (<aph@grisbi.org>)",
"Baluchiterium  (<baluchiterium@users.sf.net>)",
"Francois Terrot (<grisbi@terrot.net>)",
"Gerald Niel (<gerald.niel@grisbi.org>)",
"Gilles Morel (<g.morel@gmail.com>)",
"PMF (<ugly.duck@gmx.de>)",
"Pascal Bleser (<guru@linuxbe.org>)",
"Sylvain Glaize (<mokona@puupuu.org>)",
"Thierry Thomas (<thierry@pompo.net>)",
"Vincent Marqueton (<vincent@marqueton.com>)",
"William Ollivier (<guneeyoufix@gmail.com>)"
"\n",

_("Other"),
"Axel Rousseau (<axel584@axel584.org>) ",
"Dominique Parisot (<parisot@villey-le-sec.com>) ",
"Gerald Niel (<gerald.niel@grisbi.org>) ",
NULL };

  gchar * translators = g_strconcat(
"Alain Portal (<dionysos@grisbi.org>): ", _("English"), CSUFFIX,
"Benjamin Drieu (<bdrieu@april.org>): ", _("English"), CSUFFIX,
"Carlos M. Cámara Mora (<carcam_moceu@yahoo.es>): ", _("Spanish"), CSUFFIX,
"Daniel Cartron (<cartron@grisbi.org>): ", _("English"), CSUFFIX,
"Edwin Huijsing (<e.huijsing@fiberworld.nl>): ", _("Dutch"), CSUFFIX,
"Fabio Erculiani (<fabio.erculiani@tiscali.it>): ", _("Italian"), CSUFFIX,
"Flavio Henrique Somensi (<flavio@opens.com.br>): ", _("Brazilian Portuguese"), CSUFFIX,
"Giorgio Mandolfo (<giorgio@pollycoke.org>): ", _("Italian"), CSUFFIX,
"Martin Stromberger (<Fabiolla@aon.at>): ", _("German"), CSUFFIX,
"Ryszard Jeziorski (<rjeziorski@eagleW): ", _("Polish"), CSUFFIX,
"Achilleas Kaskamanidis (<alterna55@users.sourceforge.net>): ", _("Greek"), CSUFFIX,
NULL);

  const gchar *documenters [] = {
"Alain Portal (<aportal@univ-monpt2.fr>)",
"Daniel Cartron (<doc@grisbi.org>)",
NULL};

  const gchar *artists [] = {
"Andre Pascual (<andre@linuxgraphic.org>)",
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
                        "along with this program; if not, write to the Free Software "
                        "Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n\n"
                        "This product includes software developed by the OpenSSL Project "
                        "for use in the OpenSSL Toolkit.\n(http://www.openssl.org/)";

    /* Others info */
    gchar *comments;
    gchar *plugins;
    gchar *version_to_string;

    plugins = gsb_plugin_get_list ( );
    version_to_string = get_gtk_run_version ( );

    if ( IS_DEVELOPMENT_VERSION == 1 )
    {
        gchar *compiled_time;

        compiled_time = gsb_date_get_compiled_time ( );

        comments = g_strconcat ( _("Personal finance manager for everyone\n"),
                        plugins,
                        "\nVersion de GTK : ",
                        version_to_string,
                        "\n",
                        _("This instance of Grisbi was compiled on\n"),
                        compiled_time,
                        " ",
                        _("at"),
                        " ",
                        __TIME__,
                        NULL );

        g_free ( compiled_time );
    }
    else
        comments = g_strconcat ( _("Personal finance manager for everyone\n"),
                        plugins,
                        "\nVersion de GTK : ",
                        version_to_string,
                        NULL );

    /* Logo */
    logo = gsb_select_icon_get_logo_pixbuf ( );
    if (logo == NULL )
    {
        chemin_logo = my_strdup ( LOGO_PATH );
        logo =  gdk_pixbuf_new_from_file ( chemin_logo, NULL );

        g_free ( chemin_logo );
    }

    about = gtk_about_dialog_new ( );
    gtk_about_dialog_set_url_hook (launch_url, NULL, NULL);
    gtk_about_dialog_set_email_hook (launch_mailto, NULL, NULL);
    gtk_about_dialog_set_program_name ( GTK_ABOUT_DIALOG (about), "Grisbi" );
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

    gtk_window_set_position ( GTK_WINDOW ( about ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_window_set_resizable ( GTK_WINDOW ( about ), TRUE );
    gtk_window_set_modal ( GTK_WINDOW ( about ), TRUE );
    gtk_window_set_transient_for ( GTK_WINDOW ( about ), GTK_WINDOW ( window ) );

    g_free ( plugins );
    g_free ( version_to_string );
    g_free ( comments );

    gtk_dialog_run ( GTK_DIALOG (about)) ;

    gtk_widget_destroy (about);

    if (logo)
    g_object_unref (logo);

    return;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
