/*  Fichier qui affiche une fenetre pour faire patienter pendant les opérations longues */
/*     patienter.c */


/*     Copyright (C) 2000-2003  Cédric Auger */
/* 			cedric@grisbi.org */
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
#include "gtkcombofix.h"
#include "patienter.h"

/* on inclut les dessins */

#include "./xpm/g1.xpm"
#include "./xpm/g2.xpm"
#include "./xpm/g3.xpm"
#include "./xpm/g4.xpm"
#include "./xpm/g5.xpm"
#include "./xpm/g6.xpm"
#include "./xpm/g7.xpm"
#include "./xpm/g8.xpm"
#include "./xpm/g9.xpm"
#include "./xpm/g10.xpm"
#include "./xpm/g11.xpm"
#include "./xpm/g12.xpm"
#include "./xpm/g13.xpm"
#include "./xpm/g14.xpm"
#include "./xpm/g15.xpm"


/* ******************************************************************************************** */
/* mise_en_route_attente */
/* origine : 0 : chargement de fichier */
/* 1 : opération longue */
/* ******************************************************************************************** */

void mise_en_route_attente ( gchar *message )
{

    if ( !fenetre_patience )
    {
	/* création de la fenetre */

	GtkWidget *vbox;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *aire_dessin;

	fenetre_patience = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_policy ( GTK_WINDOW ( fenetre_patience ),
				FALSE,
				FALSE,
				TRUE );
	gtk_window_set_transient_for ( GTK_WINDOW ( fenetre_patience ),
				       GTK_WINDOW ( window ));
	gtk_widget_set_usize ( fenetre_patience,
			       200,
			       FALSE );
	g_signal_connect ( G_OBJECT ( fenetre_patience ),
			   "delete-event",
			   G_CALLBACK ( gtk_true ),
			   NULL );

	frame = gtk_frame_new ( NULL );
	gtk_container_add ( GTK_CONTAINER ( fenetre_patience ),
			    frame );
	gtk_widget_show ( frame );

	vbox = gtk_vbox_new ( FALSE,
			      5 );
	gtk_container_set_border_width ( GTK_CONTAINER ( vbox ),
					 10 );
	gtk_container_add ( GTK_CONTAINER ( frame ),
			    vbox );
	gtk_widget_show ( vbox );

	label = gtk_label_new ( _("Wait please...") );
	gtk_box_pack_start ( GTK_BOX ( vbox ),
			     label,
			     FALSE,
			     FALSE, 
			     10 );
	gtk_widget_show ( label );

	aire_dessin = gtk_hbox_new ( FALSE,
				     0 );
	gtk_box_pack_start ( GTK_BOX ( vbox ),
			     aire_dessin,
			     FALSE,
			     FALSE, 
			     10 );
	gtk_widget_show ( aire_dessin );

	image[0] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g8_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[0] );
	gtk_widget_show ( image[0] );

	image[1] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g9_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[1] );

	image[2] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g10_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[2] );

	image[3] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g11_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[3] );

	image[4] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g12_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[4] );

	image[5] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g13_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[5] );

	image[6] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g14_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[6] );

	image[7] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g15_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[7] );

	image[8] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g1_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[8] );

	image[9] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g2_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[9] );

	image[10] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g3_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[10] );

	image[11] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g4_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[11] );

	image[12] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g5_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[12] );

	image[13] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g6_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[13] );

	image[14] = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) g7_xpm ));
	gtk_container_add ( GTK_CONTAINER ( aire_dessin ),
			    image[14] );


	label_patience = gtk_label_new ( "" );
	gtk_box_pack_start ( GTK_BOX ( vbox ),
			     label_patience,
			     FALSE,
			     FALSE,
			     10 );
	gtk_widget_show ( label_patience );

	image_patience_en_cours = 0;
    }

    gtk_label_set_text ( GTK_LABEL ( label_patience ),
			 message );

    gtk_widget_show ( fenetre_patience );

    while ( g_main_iteration ( FALSE ) );

    timeout_patience = gtk_timeout_add ( 100,
					 (GtkFunction) animation_patienter,
					 NULL );
    patience_en_cours = 1;
}
/* ******************************************************************************************** */


/* ******************************************************************************************** */
void update_attente ( gchar *message )
{
    gtk_label_set_text ( GTK_LABEL ( label_patience ),
			 message );
    while ( g_main_iteration ( FALSE ) );

}
/* ******************************************************************************************** */


/* ******************************************************************************************** */
void annulation_attente ()
{
    gtk_timeout_remove ( timeout_patience );
    gtk_widget_destroy ( fenetre_patience );
    fenetre_patience = NULL;
    patience_en_cours = 0;
}
/* ******************************************************************************************** */



/* *********************************************************************************************************** */
gint animation_patienter ()
{
    gtk_widget_hide (  image[image_patience_en_cours] );
    image_patience_en_cours = (image_patience_en_cours + 1)%15;
    gtk_widget_show ( image[image_patience_en_cours] );

    while ( g_main_iteration ( FALSE ) );

    return TRUE;
}
/* *********************************************************************************************************** */


