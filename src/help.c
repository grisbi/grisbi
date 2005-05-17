/* Fichier help.c */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2003 Benjamin Drieu (bdrieu@april.org) */
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
#include "utils_buttons.h"
#include "dialog.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern gchar *chemin_logo;
extern GtkWidget *window;
/*END_EXTERN*/




/* **************************************************************************************************************************** */
void a_propos ( GtkWidget *bouton,
		gint data )
{
    GtkWidget *dialogue, *url, *label, *notebook, *vbox, *hbox, *sw, *logo;
    gint i;

#define CPREFIX "  "
#define CSUFFIX "\n"

    const gchar *auteurs = g_strconcat ( 
"\n", "<b><big>", _("Programming"), "</big></b>\n",
CPREFIX, _("Alain Portal (aportal[at]univ-monpt2.fr)"), CSUFFIX,
CPREFIX, _("Benjamin Drieu (bdrieu[at]april.org)"), CSUFFIX,
CPREFIX, _("Cedric Auger (cedric[at]grisbi.org)"), CSUFFIX,
CPREFIX, _("Francois Terrot (grisbi[at]terrot.net)"), CSUFFIX,
"\n",

"<b><big>", _("Packaging"), "</big></b>\n",
CPREFIX, _("Alain Pichon (aph[at]grisbi.org)"), CSUFFIX,
CPREFIX, _("Baluchiterium  (baluchiterium[at]users.sf.net)"), CSUFFIX,
CPREFIX, _("Francois Terrot (grisbi[at]terrot.net)"), CSUFFIX,
CPREFIX, _("Gerald Niel (gerald.niel[at]grisbi.org)"), CSUFFIX,
CPREFIX, _("PMF (ugly.duck[at]gmx.de)"), CSUFFIX,
CPREFIX, _("Pascal Bleser (guru[at]linuxbe.org)"), CSUFFIX,
CPREFIX, _("Sylvain Glaize (mokona[at]puupuu.org)"), CSUFFIX,
CPREFIX, _("Thierry Thomas (thierry[at]pompo.net)"), CSUFFIX,
CPREFIX, _("Vincent Marqueton (vincent[at]marqueton.com)"), CSUFFIX,
"\n",

"<b><big>", _("Translation"), "</big></b>\n",
CPREFIX, _("Alain Portal (dionysos[at]grisbi.org): English"), CSUFFIX,
CPREFIX, _("Benjamin Drieu (bdrieu[at]april.org): English"), CSUFFIX,
CPREFIX, _("Carlos M. Cámara Mora (carcam_moceu[at]yahoo.es): Spanish"), CSUFFIX,
CPREFIX, _("Daniel Cartron (cartron[at]grisbi.org): English"), CSUFFIX,
CPREFIX, _("Edwin Huijsing (e.huijsing[at]fiberworld.nl): Dutch"), CSUFFIX,
CPREFIX, _("Fabio Erculiani (fabio.erculiani[at]tiscali.it): Italian"), CSUFFIX,
CPREFIX, _("Flavio Henrique Somensi (flavio[at]opens.com.br): Brazilian Portuguese"), CSUFFIX,
CPREFIX, _("Giorgio Mandolfo (giorgio[at]pollycoke.org): Italian"), CSUFFIX,
CPREFIX, _("Martin Stromberger (Fabiolla[at]aon.at): German"), CSUFFIX,
CPREFIX, _("Ryszard Jeziorski (rjeziorski[at]eagle): Polish"), CSUFFIX,
"\n",

"<b><big>", _("Other"), "</big></b>\n",
CPREFIX, _("Alain Portal (aportal[at]univ-monpt2.fr): manual"), CSUFFIX,
CPREFIX, _("Andre Pascual (andre[at]linuxgraphic.org): Logo"), CSUFFIX,
CPREFIX, _("Axel Rousseau (axel584[at]axel584.org): contributor"), CSUFFIX,
CPREFIX, _("Daniel Cartron (doc[at]grisbi.org): manual"), CSUFFIX,
CPREFIX, _("Dominique Parisot (parisot[at]villey-le-sec.com): contributor"), CSUFFIX,
CPREFIX, _("Gerald Niel (gerald.niel[at]grisbi.org): administration"), CSUFFIX,

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

    const gchar *license = "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA";

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
	chemin_logo = g_strdup ( LOGO_PATH );
    logo =  gtk_image_new_from_file ( chemin_logo );
    gtk_box_pack_start ( GTK_BOX ( vbox ), logo, FALSE, FALSE, 0 );

    /* Title */
    label = gtk_label_new ( g_strconcat ( "Grisbi ", VERSION, "\n\n", NULL ));
    gtk_label_set_markup ( GTK_LABEL (label), 
			   g_strconcat ( "<span size=\"x-large\" weight=\"bold\">",

					 "Grisbi ", VERSION, "</span>", NULL ) );
    gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 0 );

    /* Resume */
    label = gtk_label_new ( _("Personal finance manager for all") );
    gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
    gtk_label_set_line_wrap ( GTK_LABEL ( label ), TRUE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), label, TRUE, TRUE, 0 );

    /* Authors */
    label = gtk_label_new ("");
    gtk_label_set_markup ( GTK_LABEL(label), auteurs );
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
