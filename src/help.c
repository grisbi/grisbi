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

  const gchar *auteur[] = { "Programmation : Cédric Auger  ( cedric@grisbi.org ) ",
			    "Manuel : Daniel Cartron ( doc@grisbi.org )",
			    "Logo : André Pascual ( andre@linuxgraphic.org )",
			    NULL };

  boite = gnome_about_new ( "Grisbi ",
			    VERSION,
			    "License GPL ( General Public License )",
			    (const gchar **) auteur,
			    "Programme de gestion financière personnelle",
			    NULL );

  url = gnome_href_new ( "http://www.grisbi.org",
			    "WEB :  http://www.grisbi.org " );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       url,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( url );

  url = gnome_href_new ( "mailto:devel-subscribe@grisbi.org",
			 "Inscription à la liste de développement" );
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
			 "Inscription à la liste d'information" );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       url,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( url );

  label = gtk_label_new ( "(Annonces de sortie de nouvelle version)" );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  url = gnome_href_new ( "http://www.grisbi.org/telechargement/doc/illustre/grisbi.html",
			 "Documentation en ligne" );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       url,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( url );

  label = gtk_label_new ( "(Documentation illustrée sur le site de Grisbi)" );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( boite ) -> vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );

  if ( data == 1 )
    gnome_dialog_set_parent ( GNOME_DIALOG ( boite ),
			      GTK_WINDOW ( window));
  else
    gtk_window_set_position ( GTK_WINDOW ( boite ),
			      GTK_WIN_POS_CENTER );

  gtk_widget_show ( boite );
}
/* **************************************************************************************************************************** */
