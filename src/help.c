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
  GtkWidget *dialogue;
  GtkWidget *url;
  GtkWidget *label;
  GtkWidget *notebook;
  GtkWidget *vbox;
  GtkWidget *hbox;
  gint i;

  const gchar *generalites = _("Grisbi est un logiciel vous permettant de gérer votre comptabilité personnelle sous Linux.\nLe principe de base d'un tel logiciel est de vous permettre de classer vos opérations financières, quelles qu'elles soient, de façon simple et intuitive, afin de pouvoir les exploiter au mieux en fonction de vos besoins.\nGrisbi a pris le parti de la simplicité et de l'efficacité pour un usage de base, sans toutefois exclure la sophistication nécessaire à un usage avancé. Les fonctionnalités futures tenteront toujours de respecter ces critères.");

  const gchar *auteurs[] = { _("Cédric Auger (cedric@grisbi.org) : Programming"),
			    _("Daniel Cartron (doc@grisbi.org) : Manual, website, csultancy"),
			    _("Benjamin Drieu (bdrieu@april.org) : Programming, Debian packaging"),
			    _("Gérald Niel (gerald.niel@grisbi.org) : RPM packaging, website"),
			    _("André Pascual (andre@linuxgraphic.org) : Logo"),
			    NULL };
  const gchar *traducteurs[] = { "Alain Portal (aportal@univ-montp2.fr) : English",
				 NULL };
  const gchar *liens[] = { COLON(_("Website")),
			   "http://www.grisbi.org/",
			   _("http://www.grisbi.org/"),
			   _("Development list (discuss, participate, criticize, ...) : "),
			   "mailto:devel-subscribe@grisbi.org",
			   _("subscribe"),
			   COLON(_("Bug report")),
			   "http://www.grisbi.org/bugtracking",
			   "http://www.grisbi.org/bugtracking",
			   _("Information list (new releases announces, ...) : "),
			   "mailto:infos-subscribe@grisbi.org",
			   _("subscribe"),
			   COLON(_("Grisbi documentation")),
			   "http://www.grisbi.org/modules.php?name=Documentation",
			   _("http://www.grisbi.org/modules.php?name=Documentation"),
			   NULL };

  const gchar *license = "This program is free software; you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation; either version 2 of the License, or\n(at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.You should have received a copy of the GNU General Public License\nalong with this program; if not, write to the Free Software\nFoundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA";

  dialogue = gnome_dialog_new ( _("About..."),
				GNOME_STOCK_BUTTON_APPLY,
				NULL );
  gtk_window_set_transient_for ( GTK_WINDOW ( dialogue ),
				 GTK_WINDOW ( window ) );

  notebook = gtk_notebook_new ();
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialogue ) -> vbox ),
		       notebook,
		       TRUE,
		       TRUE,
		       0 );
  gtk_widget_show ( notebook );


  /* mise en forme de l'onglet de généralités */

  vbox = gtk_vbox_new ( FALSE,
			5 );
  gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
				   10 );
  gtk_widget_show ( vbox );

  label = gtk_label_new ( g_strconcat ( "Grisbi ",
					VERSION,
					"\n\n",
					NULL ));
  gtk_box_pack_start ( GTK_BOX ( vbox ),
		       label,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( label );


  label = gtk_label_new ( generalites );
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
			     gtk_label_new ( _("Generalities")));

  /* mise en forme de l'onglet auteurs */


  vbox = gtk_vbox_new ( FALSE,
			5 );
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

      url = gnome_href_new ( liens[i+1],
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
/*   gtk_label_set_line_wrap ( GTK_LABEL ( label ), */
/* 			    TRUE ); */
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

  gnome_dialog_run_and_close ( GNOME_DIALOG ( dialogue ));

}
/* **************************************************************************************************************************** */
