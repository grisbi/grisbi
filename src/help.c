/* Fichier help.c */


/*     Copyright (C) 2000  Cédric Auger */
/* 			cedric@grisbi.org */
/* 		 	http://www.grisbi.org */

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
#include "en_tete.h"


/* **************************************************************************************************************************** */
void a_propos ( GtkWidget *bouton,
		gint data )
{
  GtkWidget *boite;
  GtkWidget *url;
  GtkWidget *label;

  const gchar *auteur[] = { "Auger Cédric ( cedric@grisbi.org ) : ",
			    _("Programmation"),
			    "Cartron Daniel ( doc@grisbi.org ) : ",
			    _("Écriture de documentation, site Internet, conseils en comptabilité et en ergonomie"),
			    "Drieu Benjamin ( bdrieu@april.org ) : ",
			    _("Programmation, packaging Debian"),
			    "Niel Gérald ( gerald.niel@grisbi.org ) : ",
			    _("Site Internet, packaging RPM"),
			    "Pascual André ( andre@linuxgraphic.org ) : ",
			    _("Logo"),
			    NULL };

  boite = gnome_about_new ( _("Grisbi"),
			    VERSION,
			    _("License GNU GPL"),
			    (const gchar **) auteur,
			    _("Programme de gestion financière personnelle"),
			    NULL );

  url = gnome_href_new ( "http://www.grisbi.org/",
			 _("WEB : http://www.grisbi.org/") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       url,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( url );

  url = gnome_href_new ( "mailto:devel-subscribe@grisbi.org",
			 _("Inscription à la liste de développement") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       url,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( url );

  url = gnome_href_new ( "http://www.grisbi.org/bugtracking",
			 "Signaler un bug: http://www.grisbi.org/bugtracking" );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
 		       url,
 		       FALSE,
 		       FALSE,
 		       0 );
  gtk_widget_show ( url );

  url = gnome_href_new ( "mailto:infos-subscribe@grisbi.org",
			 _("Inscription à la liste d'information") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       url,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( url );

  label = gtk_label_new ( _("(Annonces de sortie de nouvelle version)") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  url = gnome_href_new ( "http://www.grisbi.org/modules.php?name=Manuel",
			 _("Documentation en ligne") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       url,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( url );

  label = gtk_label_new ( _("(Documentation illustrée sur le site de Grisbi)") );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  gnome_dialog_set_parent ( GNOME_DIALOG ( boite ),
			    GTK_WINDOW ( window));

  gtk_widget_show ( boite );
}
/* **************************************************************************************************************************** */
