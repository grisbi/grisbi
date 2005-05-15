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
#include "structures.h"
#include "variables-extern.c"
#include "help.h"

#include "utils.h"




/* **************************************************************************************************************************** */
void a_propos ( GtkWidget *bouton,
		gint data )
{
    GtkWidget *dialogue;
    GtkWidget *url;
    GtkWidget *label;
    GtkWidget *notebook;
    GtkWidget *vbox;
    GtkWidget *hbox;
    gint i;

    const gchar *generalites = _("Grisbi is a personnal accounting application for Linux\nSuch a program allows you to sort your financial transactions, whatever they are, in a convenient and intuitive way.  Thus, you will be able to manage them as to fit your needs the more you can.\nGrisbi aims both at simplicty and efficiency for common use, and at a powerful use for power users.  We will always try to respect this as features will come.");

    const gchar *auteurs[] = { 
      _("Alain Pichon (aph[at]grisbi.org): RHEL and Fedora packaging"),
      _("Alain Portal <aportal AT univ-monpt2 DOT fr>: Programming, manual"),
      _("Andre Pascual (andre[at]linuxgraphic.org): Logo"),
      _("Axel Rousseau (axel584[at]axel584.org): contributor"),
      _("Baluchiterium  (baluchiterium[at]users.sf.net): Slackware packaging"),
      _("Benjamin Drieu (bdrieu[at]april.org): Programming, Debian packaging"),
      _("Cedric Auger (cedric[at]grisbi.org): Programming"),
      _("Daniel Cartron (doc[at]grisbi.org): Manual, website, consultancy"),
      _("Dominique Parisot (parisot[at]villey-le-sec.com): contributor"),
      _("Francois Terrot (grisbi[at]terrot.net): Windows packaging and port, programming"),
      _("Gerald Niel (gerald.niel[at]grisbi.org): Darwin packaging, administration"),
      _("PMF (ugly.duck[at]gmx.de): Zaurus packaging"),
      _("Pascal Bleser (guru[at]linuxbe.org): SuSE packaging"),
      _("Sylvain Glaize (mokona[at]puupuu.org): Mandrake packaging"),
      _("Thierry Thomas (thierry[at]pompo.net): FreeBSD packaging"),
      _("Vincent Marqueton (vincent[at]marqueton.com): Fink packaging"),
      NULL };
    const gchar *traducteurs[] = { 
      _("Alain Portal (dionysos[at]grisbi.org): English"),
      _("Benjamin Drieu (bdrieu[at]april.org): English"),
      _("Carlos M. Cámara Mora (carcam_moceu[at]yahoo.es): Spanish"),
      _("Daniel Cartron (cartron[at]grisbi.org): English"),
      _("Edwin Huijsing (e.huijsing[at]fiberworld.nl): Dutch"),
      _("Fabio Erculiani (fabio.erculiani[at]tiscali.it): Italian"),
      _("Flavio Henrique Somensi (flavio[at]opens.com.br): Brazilian Portuguese"),
      _("Giorgio Mandolfo (giorgio[at]pollycoke.org): Italian"),
      _("Martin Stromberger (Fabiolla[at]aon.at): German"),
      _("Ryszard Jeziorski (r.jeziorski[at]dart.kielce.pl): Polish"),
      NULL };
    const gchar *liens[] = { 
                COLON(_("Website")),
		"http://www.grisbi.org/",
		"http://www.grisbi.org/",
		_("Development list (discuss, participate, criticize, ...): "),
		"http://lists.sourceforge.net/lists/listinfo/grisbi-devel",
		_("subscribe"),
		COLON(_("Bug report")),
		"http://grisbi.tuxfamily.org/mantis/",
		"http://grisbi.tuxfamily.org/mantis/",
		_("Information list (new releases announces, ...): "),
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
				     notebook,
				     TRUE,
				     TRUE,
				     0 );
		gtk_widget_show ( notebook );


		/* mise en forme de l'onglet de généralités */

		vbox = gtk_vbox_new ( FALSE, 5 );
		gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 10 );
		gtk_widget_show ( vbox );

		label = gtk_label_new ( g_strconcat ( "Grisbi ", VERSION, "\n\n", NULL ));
		gtk_label_set_markup ( GTK_LABEL (label), 
				       g_strconcat ( "<span size=\"large\" weight=\"bold\">",

						     "Grisbi ", VERSION, "</span>", NULL ) );

		gtk_box_pack_start ( GTK_BOX ( vbox ), label,
				     FALSE, FALSE, 0 );
		gtk_widget_show ( label );

		label = gtk_label_new ( generalites );
		gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
		gtk_label_set_line_wrap ( GTK_LABEL ( label ),
					  TRUE );
		gtk_box_pack_start ( GTK_BOX ( vbox ),
				     label,
				     TRUE,
				     TRUE,
				     0 );
		gtk_widget_show ( label );

		gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
					   vbox,
					   gtk_label_new ( _("About Grisbi")));

		/* mise en forme de l'onglet auteurs */


		vbox = gtk_vbox_new ( FALSE, 6 );
		gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
						 10 );
		gtk_widget_show ( vbox );

		i=0;

		while ( auteurs[i] )
		{
		    hbox = gtk_hbox_new ( FALSE, 
					  0 );
		    gtk_box_pack_start ( GTK_BOX ( vbox ),
					 hbox,
					 FALSE,
					 FALSE,
					 0 );
		    gtk_widget_show ( hbox );

		    label = gtk_label_new ( auteurs[i] );
		    gtk_box_pack_start ( GTK_BOX ( hbox ),
					 label,
					 FALSE,
					 FALSE,
					 0 );
		    gtk_widget_show ( label );
		    i++;
		}

		gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
					   vbox,
					   gtk_label_new ( _("Authors")));


		/* mise en forme de l'onglet traduction */


		vbox = gtk_vbox_new ( FALSE,
				      5 );
		gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
						 10 );
		gtk_widget_show ( vbox );

		i=0;

		while ( traducteurs[i] )
		{
		    hbox = gtk_hbox_new ( FALSE, 
					  0 );
		    gtk_box_pack_start ( GTK_BOX ( vbox ),
					 hbox,
					 FALSE,
					 FALSE,
					 0 );
		    gtk_widget_show ( hbox );

		    label = gtk_label_new ( traducteurs[i] );
		    gtk_box_pack_start ( GTK_BOX ( hbox ),
					 label,
					 FALSE,
					 FALSE,
					 0 );
		    gtk_widget_show ( label );
		    i++;
		}

		gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
					   vbox,
					   gtk_label_new ( _("Translators")));


		/* mise en forme de l'onglet liens */


		vbox = gtk_vbox_new ( FALSE,
				      5 );
		gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
						 10 );
		gtk_widget_show ( vbox );

		i=0;

		while ( liens[i] )
		{
		    hbox = gtk_hbox_new ( FALSE,
					  5 );
		    gtk_box_pack_start ( GTK_BOX ( vbox ),
					 hbox,
					 FALSE,
					 FALSE,
					 0 );
		    gtk_widget_show ( hbox );

		    label = gtk_label_new ( liens[i] );
		    gtk_box_pack_start ( GTK_BOX ( hbox ),
					 label,
					 FALSE,
					 FALSE,
					 0 );
		    gtk_widget_show ( label );

		    url = cree_bouton_url ( liens[i+1],
					    liens[i+2] );
		    gtk_box_pack_start ( GTK_BOX ( hbox ),
					 url,
					 FALSE,
					 FALSE,
					 0 );
		    gtk_widget_show ( url );

		    i = i + 3;
		}

		gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
					   vbox,
					   gtk_label_new ( _("Links")));


		/* mise en forme de l'onglet de license */

		vbox = gtk_vbox_new ( FALSE,
				      5 );
		gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
						 10 );
		gtk_widget_show ( vbox );

		label = gtk_label_new ( license );
		gtk_label_set_selectable ( GTK_LABEL ( label ), TRUE );
		gtk_label_set_line_wrap ( GTK_LABEL ( label ),
					  TRUE );
		gtk_box_pack_start ( GTK_BOX ( vbox ),
				     label,
				     FALSE,
				     FALSE,
				     0 );
		gtk_widget_show ( label );

		gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
					   vbox,
					   gtk_label_new ( _("License")));

		gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook ),
					0 );

		gtk_dialog_run ( GTK_DIALOG ( dialogue ));
		gtk_widget_destroy ( dialogue );

}
/* **************************************************************************************************************************** */


