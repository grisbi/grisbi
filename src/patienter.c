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


#define START_INCLUDE
#include "patienter.h"
#include "utils.h"
#define END_INCLUDE

#define START_STATIC
#define END_STATIC



GtkWidget *label_patience;
GtkWidget *fenetre_patience;
gint patience_en_cours;
gint idle_stoppe;

#define START_EXTERN
extern GtkWidget *window;
#define END_EXTERN



/* ******************************************************************************************** */
/* mise_en_route_attente */
/* origine : 0 : chargement de fichier */
/* 1 : opération longue */
/* ******************************************************************************************** */

void mise_en_route_attente ( gchar *message )
{
    GdkPixbufAnimation *pixbuf_animation;
    GtkWidget *animation;
    GtkWidget *vbox;
    GtkWidget *frame;
    GtkWidget *label;

    if ( DEBUG )
	printf ( "mise_en_route_attente %s\n", message );

    /*     on stoppe l'idle */

    idle_stoppe = termine_idle ();

    /*     s'il y a déjà une attente en cours, on l'update */

    if ( patience_en_cours )
    {
	update_attente ( message );
	return;
    }

    fenetre_patience = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
    gtk_window_set_position ( GTK_WINDOW ( fenetre_patience),
			      GTK_WIN_POS_CENTER );
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

    if ( etat.fichier_animation_attente )
    {
	pixbuf_animation = gdk_pixbuf_animation_new_from_file ( etat.fichier_animation_attente,
								NULL );
	if ( pixbuf_animation )
	{
	    animation = gtk_image_new_from_animation ( pixbuf_animation );
	    gtk_box_pack_start ( GTK_BOX ( vbox ),
				 animation,
				 FALSE,
				 FALSE, 
				 10 );
	    gtk_widget_show ( animation );
	}
    }

    label_patience = gtk_label_new ( message );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 label_patience,
			 FALSE,
			 FALSE,
			 10 );
    gtk_widget_show ( label_patience );

    gtk_widget_show ( fenetre_patience );

    update_ecran ();

    patience_en_cours = 1;
}
/* ******************************************************************************************** */


/* ******************************************************************************************** */
void update_attente ( gchar *message )
{
    if ( DEBUG )
	printf ( "update_attente %s\n", message );

    /*     s'il n'y a pas de patience en cours, on la débute */

    if ( !patience_en_cours )
    {
	mise_en_route_attente ( message );
	return;
    }

    if ( GTK_IS_LABEL ( label_patience ))
    {
	gtk_label_set_text ( GTK_LABEL ( label_patience ),
			     message );
	update_ecran ();
    }
}
/* ******************************************************************************************** */


/* ******************************************************************************************** */
void annulation_attente ()
{
    if ( DEBUG )
	printf ( "annulation_attente\n" );

    patience_en_cours = 0;

    if ( GTK_IS_WIDGET ( fenetre_patience ))
    {
	gtk_widget_destroy ( fenetre_patience );
	fenetre_patience = NULL;
    }

    if ( idle_stoppe )
	demarrage_idle ();
}
/* ******************************************************************************************** */

