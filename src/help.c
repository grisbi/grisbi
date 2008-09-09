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
#include "./utils_buttons.h"
#include "./dialog.h"
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
    GtkWidget *dialogue, *url, *label, *notebook, *vbox, *hbox, *sw, *logo;
    gint i;

#define CPREFIX "  "
#define CSUFFIX "\n"

    gchar *auteurs = g_strconcat ( 
"\n", "<b><big>", _("Programming"), "</big></b>\n",
CPREFIX, "Benjamin Drieu (bdrieu[at]april.org)", CSUFFIX,
CPREFIX, "Cedric Auger (cedric[at]grisbi.org)", CSUFFIX,
CPREFIX, "Francois Terrot (grisbi[at]terrot.net)", CSUFFIX,
"\n",

"<b><big>", _("Packaging"), "</big></b>\n",
CPREFIX, "Alain Pichon (aph[at]grisbi.org)", CSUFFIX,
CPREFIX, "Baluchiterium  (baluchiterium[at]users.sf.net)", CSUFFIX,
CPREFIX, "Francois Terrot (grisbi[at]terrot.net)", CSUFFIX,
CPREFIX, "Gerald Niel (gerald.niel[at]grisbi.org)", CSUFFIX,
CPREFIX, "PMF (ugly.duck[at]gmx.de)", CSUFFIX,
CPREFIX, "Pascal Bleser (guru[at]linuxbe.org)", CSUFFIX,
CPREFIX, "Sylvain Glaize (mokona[at]puupuu.org)", CSUFFIX,
CPREFIX, "Thierry Thomas (thierry[at]pompo.net)", CSUFFIX,
CPREFIX, "Vincent Marqueton (vincent[at]marqueton.com)", CSUFFIX,
"\n",

"<b><big>", _("Translation"), "</big></b>\n",
CPREFIX, "Alain Portal (dionysos[at]grisbi.org): ", _("English"), CSUFFIX,
CPREFIX, "Benjamin Drieu (bdrieu[at]april.org): ", _("English"), CSUFFIX,
CPREFIX, "Carlos M. Cámara Mora (carcam_moceu[at]yahoo.es): ", _("Spanish"), CSUFFIX,
CPREFIX, "Daniel Cartron (cartron[at]grisbi.org): ", _("English"), CSUFFIX,
CPREFIX, "Edwin Huijsing (e.huijsing[at]fiberworld.nl): ", _("Dutch"), CSUFFIX,
CPREFIX, "Fabio Erculiani (fabio.erculiani[at]tiscali.it): ", _("Italian"), CSUFFIX,
CPREFIX, "Flavio Henrique Somensi (flavio[at]opens.com.br): ", _("Brazilian Portuguese"), CSUFFIX,
CPREFIX, "Giorgio Mandolfo (giorgio[at]pollycoke.org): ", _("Italian"), CSUFFIX,
CPREFIX, "Martin Stromberger (Fabiolla[at]aon.at): ", _("German"), CSUFFIX,
CPREFIX, "Ryszard Jeziorski (rjeziorski[at]eagle): ", _("Polish"), CSUFFIX,
"\n",

"<b><big>", _("Other"), "</big></b>\n",
CPREFIX, "Alain Portal (aportal[at]univ-monpt2.fr): ", _("manual"), CSUFFIX,
CPREFIX, "Andre Pascual (andre[at]linuxgraphic.org): ", _("logo"), CSUFFIX,
CPREFIX, "Axel Rousseau (axel584[at]axel584.org): ", _("contributor"), CSUFFIX,
CPREFIX, "Daniel Cartron (doc[at]grisbi.org): ", _("manual"), CSUFFIX,
CPREFIX, "Dominique Parisot (parisot[at]villey-le-sec.com): ", _("contributor"), CSUFFIX,
CPREFIX, "Gerald Niel (gerald.niel[at]grisbi.org): ", _("administration"), CSUFFIX,

NULL );	

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

    dialogue = gtk_dialog_new_with_buttons ( _("About..."),
					     GTK_WINDOW (window),
					     GTK_DIALOG_MODAL,
					     GTK_STOCK_CLOSE,0,
					     NULL );

    notebook = gtk_notebook_new ();
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG ( dialogue ) -> vbox ),
			 notebook, TRUE, TRUE, 0 );

    /* Generalities */
    sw = gtk_scrolled_window_new ( NULL, NULL );    
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( sw ),
				     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 6 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( sw ), vbox );

    /* Logo */
    if ( !chemin_logo || !strlen ( chemin_logo ))
	chemin_logo = my_strdup ( LOGO_PATH );
    logo =  gtk_image_new_from_file ( chemin_logo );
    gtk_box_pack_start ( GTK_BOX ( vbox ), logo, FALSE, FALSE, 0 );

    /* Title */
    gchar* tmpstr = g_strconcat ( "Grisbi ", VERSION, "\n\n", NULL );
    label = gtk_label_new ( tmpstr );
    g_free ( tmpstr );
    gchar* plugin_list = gsb_plugin_get_list();
    gchar* buffer = g_strconcat ( "<span size=\"x-large\" weight=\"bold\">",
					 "Grisbi ", VERSION, "</span>\n", 
					 "<span size=\"small\">",
					 plugin_list, 
					 "</span>", NULL );
    gtk_label_set_markup ( GTK_LABEL (label), buffer);
    g_free(buffer);
    g_free(plugin_list);
    gtk_label_set_justify ( GTK_LABEL ( label ), GTK_JUSTIFY_CENTER );
    gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 0 );

    /* Resume */
    label = gtk_label_new ( _("Personal finance manager for everyone") );
    gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
    gtk_label_set_line_wrap ( GTK_LABEL ( label ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, TRUE, TRUE, 0 );

#if ! GTK_CHECK_VERSION(2,10,0)
    /* Warn about obsolete dependencies */
    label = gtk_label_new ( _("This version of Grisbi does not support print feature.  Version of GTK+ used is obsolete.") );
    gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
    gtk_label_set_line_wrap ( GTK_LABEL ( label ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, TRUE, TRUE, 0 );
#endif

    /* Authors */
    label = gtk_label_new (NULL);
    gchar** tmparray = g_strsplit ( auteurs, "[at]", 0 );
    tmpstr = g_strjoinv ( "@", tmparray );
    g_free ( auteurs );
    g_strfreev(tmparray);
    gtk_label_set_markup ( GTK_LABEL(label), tmpstr );
    g_free ( tmpstr );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, TRUE, TRUE, 0 );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), sw,
			       gtk_label_new ( _("About Grisbi")));


    /* mise en forme de l'onglet liens */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 10 );

    i=0;

    while ( liens[i] )
	{
	    hbox = gtk_hbox_new ( FALSE, 5 );
	    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

	    label = gtk_label_new ( liens[i] );
	    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

	    url = cree_bouton_url ( liens[i+1], liens[i+2] );
	    gtk_box_pack_start ( GTK_BOX ( hbox ), url, FALSE, FALSE, 0 );

	    i = i + 3;
	}

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), vbox, 
			       gtk_label_new ( _("Links")));


    /* mise en forme de l'onglet de license */ 
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 10 );

    label = gtk_label_new ( license );
    gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
    gtk_label_set_line_wrap ( GTK_LABEL ( label ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 0 );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), vbox,
			       gtk_label_new ( _("License")));

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook ), 0 );

    gtk_widget_show_all ( dialogue );

    gtk_dialog_run ( GTK_DIALOG ( dialogue ));
    gtk_widget_destroy ( dialogue );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
