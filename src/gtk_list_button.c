/* ************************************************************************** */
/* Button with an icon and a text, with no particular size request.	      */
/*                                                                            */
/*     Copyright (C)    1995-1997 Peter Mattis, Spencer Kimball and           */
/*			          Josh MacDonald			      */
/*     	                2004      Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
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

#include <stdio.h>
#include "gtk_list_button.h"

#define CHILD_SPACING     1

#include "xpm/book-closed.xpm"
#include "xpm/book-open.xpm"

#include "utils.h"

/* Function protypes */
static void gtk_list_button_class_init ( GtkListButtonClass * );
static void gtk_list_button_init ( GtkListButton * );
static void gtk_list_button_size_request (GtkWidget *, GtkRequisition *);
static gboolean gtk_list_button_destroy ( GtkListButton * );
static gboolean gtk_list_button_press_event ( GtkWidget *button,
					      GdkEventButton *ev   );
static gboolean gtk_list_button_release_event ( GtkWidget *button,
						GdkEventButton *ev );
static gboolean leaving_button_while_dragging ( GtkWidget *button,
						GdkEventCrossing *ev  );



/** Contain a group of buttons, to store the one selected and thus,
  deselect is when another one is clicked.  Somewhat ugly, but I
  don't expect this to be widely used, so... */
GtkListButton *groups[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* id du signal appelé pendant un drag */

gulong id_signal_drag_and_drop;

/* adresse du bouton en cours de drag */

GtkWidget *dragging_button = NULL;

/* utilisé pour différencier un click d'un drag quand le drag est actif */

gint moved;

/* utilisé à la création du list_button pour passer can_drag à la fonction gtk_list_button_init */

gint list_button_can_drag;


/* Functions */

guint gtk_list_button_get_type ( void )
{
    static guint gtk_list_button_type = 0;

    if ( !gtk_list_button_type )
    {
	static const GtkTypeInfo gtk_list_button_info = {
	    "GtkListButton",
	    sizeof (GtkListButton),
	    sizeof (GtkListButtonClass),
	    (GtkClassInitFunc) gtk_list_button_class_init,
	    (GtkObjectInitFunc) gtk_list_button_init,
	    NULL,
	    NULL,
	    (GtkClassInitFunc) NULL
	};

	gtk_list_button_type = gtk_type_unique ( gtk_button_get_type(),
						 &gtk_list_button_info );
    }

    return ( gtk_list_button_type );

}



static void gtk_list_button_class_init ( GtkListButtonClass *klass )
{
    GtkWidgetClass *widget_class;

    widget_class = (GtkWidgetClass*) klass;
    widget_class->size_request = gtk_list_button_size_request;

    klass -> reordered = NULL;

    g_signal_new ( "reordered",
		   G_OBJECT_CLASS_TYPE (G_OBJECT_CLASS (klass)),
		   G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		   G_STRUCT_OFFSET (GtkListButtonClass, reordered),
		   NULL,
		   NULL,
		   g_cclosure_marshal_VOID__VOID,
		   G_TYPE_NONE,
		   0 );
}



static void gtk_list_button_init ( GtkListButton *list_button )
{
    list_button -> hbox = gtk_hbox_new ( FALSE, 6 );

    list_button -> closed_icon = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) book_closed_xpm ) );
    gtk_widget_set_usize ( list_button -> closed_icon, 24, 24 );

    list_button -> box = gtk_vbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( list_button -> hbox ), list_button -> box,
			 FALSE, FALSE, 0 );

    gtk_container_add ( GTK_CONTAINER(list_button -> box), list_button->closed_icon);

    list_button -> open_icon = gtk_image_new_from_pixbuf ( gdk_pixbuf_new_from_xpm_data ( (const gchar **) book_open_xpm ));
    gtk_widget_set_usize ( list_button -> open_icon, 24, 24 );

    list_button -> label = gtk_label_new ("") ;
    gtk_label_set_line_wrap ( GTK_LABEL(list_button->label), TRUE );
    gtk_box_pack_start ( GTK_BOX(list_button -> hbox), list_button -> label,
			 FALSE, FALSE, 0 );

    gtk_container_add ( GTK_CONTAINER(list_button), list_button -> hbox );
    gtk_button_set_relief ( GTK_BUTTON (list_button), GTK_RELIEF_NONE );

    g_signal_connect ( GTK_WIDGET(list_button), "destroy", 
		       G_CALLBACK ( gtk_list_button_destroy ), NULL );
    g_signal_connect ( GTK_BUTTON(list_button), "clicked", 
		       G_CALLBACK ( gtk_list_button_clicked ), NULL );

    /*     si on a rendu le drag possible, on associe les event */

    if ( list_button_can_drag )
    {
	g_signal_connect ( GTK_WIDGET(list_button), "button-press-event", 
			   G_CALLBACK ( gtk_list_button_press_event ), NULL );
	g_signal_connect ( GTK_WIDGET(list_button), "button-release-event", 
			   G_CALLBACK ( gtk_list_button_release_event ), NULL );
    }


    gtk_widget_show ( list_button -> hbox );
    gtk_widget_show ( list_button -> box );
    gtk_widget_show ( list_button -> closed_icon );
    gtk_widget_show ( list_button -> label );
}



GtkWidget *gtk_list_button_new ( gchar * text, int group, int can_drag, gpointer data )
{
    GtkListButton *list_button;

    /* FIXME : comme je sais pas comment passer l'argument can_drag à gtk_list_button_init */
    /* 	    je le mets en variable globale */

    list_button_can_drag = can_drag;

    list_button = g_object_new (GTK_TYPE_LIST_BUTTON, NULL);

    gtk_list_button_set_name ( list_button, text );
    list_button -> group = group;

    list_button -> data = data;

    return ( GTK_WIDGET ( list_button ) );
}



static void gtk_list_button_size_request (GtkWidget      *widget,
					  GtkRequisition *requisition)
{
    GtkButton *button = GTK_BUTTON (widget);
    gint focus_width;
    gint focus_pad;

    gtk_widget_style_get (GTK_WIDGET (widget),
			  "focus-line-width", &focus_width,
			  "focus-padding", &focus_pad,
			  NULL);

    requisition->width = (GTK_CONTAINER (widget)->border_width + CHILD_SPACING +
			  GTK_WIDGET (widget)->style->xthickness) * 2;
    requisition->height = (GTK_CONTAINER (widget)->border_width + CHILD_SPACING +
			   GTK_WIDGET (widget)->style->ythickness) * 2;

    if (GTK_BIN (button)->child && GTK_WIDGET_VISIBLE (GTK_BIN (button)->child))
    {
	GtkRequisition child_requisition;

	gtk_widget_size_request (GTK_BIN (button)->child, &child_requisition);

	requisition->width += child_requisition.width;
	requisition->height += child_requisition.height;
    }

    /*   requisition->width += 2 * (focus_width + focus_pad); */
    requisition->width = -1;
    requisition->height += 2 * (focus_width + focus_pad);
}



void gtk_list_button_set_name ( GtkListButton * list_button, gchar * name )
{
    if ( name )
	gtk_label_set_text ( GTK_LABEL (list_button -> label), name );
}



gboolean gtk_list_button_clicked ( GtkButton *button )
{
    GtkListButton * listbutton;

    listbutton = groups [ GTK_LIST_BUTTON(button) -> group ];
    if ( listbutton && GTK_IS_WIDGET(listbutton) )
    {
	gtk_widget_hide ( listbutton -> open_icon );
	gtk_widget_show ( listbutton -> closed_icon );
	g_object_ref ( listbutton -> open_icon );
	gtk_container_add ( GTK_CONTAINER (listbutton -> box), 
			    listbutton -> closed_icon );
	gtk_container_remove ( GTK_CONTAINER (listbutton -> box), 
			       listbutton -> open_icon );
    }

    listbutton = GTK_LIST_BUTTON(button);

    gtk_widget_hide ( listbutton -> closed_icon );
    gtk_widget_show ( listbutton -> open_icon );
    g_object_ref ( listbutton -> closed_icon );
    gtk_container_add ( GTK_CONTAINER (listbutton -> box), 
			listbutton -> open_icon );
    gtk_container_remove ( GTK_CONTAINER (listbutton -> box), 
			   listbutton -> closed_icon );

    groups [ GTK_LIST_BUTTON(button) -> group ] = listbutton;

    return FALSE;
}


void gtk_list_button_close ( GtkButton *button )
{
    GtkListButton * listbutton;

    listbutton = groups [ GTK_LIST_BUTTON(button) -> group ];
    if ( listbutton && GTK_IS_WIDGET(listbutton) )
    {
	gtk_widget_hide ( listbutton -> open_icon );
	gtk_widget_show ( listbutton -> closed_icon );
	g_object_ref ( listbutton -> open_icon );
	gtk_container_add ( GTK_CONTAINER (listbutton -> box), 
			    listbutton -> closed_icon );
	gtk_container_remove ( GTK_CONTAINER (listbutton -> box), 
			       listbutton -> open_icon );
    }
    groups [ GTK_LIST_BUTTON(button) -> group ] = NULL;
}



gboolean gtk_list_button_destroy ( GtkListButton * listbutton )
{
    if ( listbutton == groups [ listbutton -> group ] )
    {
	groups [ listbutton -> group ] = NULL;
    }  
    return ( FALSE );
}



/* *********************************************************************************************************** */
/* cette fonction est appelée lorsqu'on presse un button de compte */
/* elle permet de débuter un drag and drop  */
/* *********************************************************************************************************** */
gboolean gtk_list_button_press_event ( GtkWidget *button,
				       GdkEventButton *ev )
{
    GList *liste_tmp;


    if ( dragging_button )
	gtk_list_button_release_event ( button,
					ev );
    
    /*     on récupère maintenant le bouton qu'on va dragger */

    dragging_button = button;

    /* pour que le drag fonctionne bien, les boutons doivent être accolés, ce qu'on fait */
    /*     ici, et s'ils ne le sont pas on met un warning */

    if ( gtk_box_get_spacing ( GTK_BOX ( dragging_button -> parent )))
    {
	/* 	le spacing de la vbox est > 0, on le met à 0 */

	gtk_box_set_spacing ( GTK_BOX ( dragging_button -> parent ),
			      0 );
	printf ( "Warning : the spacing of the parent's box was not 0.\nIt is necessary to permit to drag the buttons\nIt's done.\n" );
    }

    /*     on fait le tour des fils */

    liste_tmp = GTK_BOX ( dragging_button -> parent ) -> children;

    while ( liste_tmp )
    {
	GtkBoxChild *box_child;
	gboolean expand;
	gboolean fill;
	guint padding;
	GtkPackType pack_type;

	box_child = liste_tmp -> data;

	gtk_box_query_child_packing ( GTK_BOX ( dragging_button -> parent ),
				      box_child -> widget,
				      &expand,
				      &fill,
				      &padding,
				      &pack_type );
	if ( padding )
	{
	    printf ( "Warning : the padding of the button n°%d was not 0.\nIt is necessary to permit to drag the buttons\nIt's done.\n",
		     g_list_position ( GTK_BOX ( dragging_button -> parent ) -> children,
				       liste_tmp ));
	    padding = 0;
	    gtk_box_set_child_packing ( GTK_BOX ( dragging_button -> parent ),
					box_child -> widget,
					expand,
					fill,
					padding,
					pack_type );
	}
	liste_tmp = liste_tmp -> next;
    }
    
    id_signal_drag_and_drop = g_signal_connect ( G_OBJECT ( button ),
						 "leave-notify-event",
						 G_CALLBACK ( leaving_button_while_dragging ),
						 NULL );

    gdk_pointer_grab ( button ->  window,
		       TRUE,
		       GDK_LEAVE_NOTIFY_MASK ||
		       GDK_BUTTON_RELEASE_MASK,
		       button ->  window,
		       NULL,
		       ev -> time );

    /*     moved est utilisé pour fair la différence entre un click et un grab */
    /* 	il sera mis à 1 si on déplace le compte  */

    moved = 0;

    return FALSE;
}
/* *********************************************************************************************************** */

				




/* *********************************************************************************************************** */
/* cette fonction est appelée lorsqu'on lache un button de compte */
/* elle permet de finir un drag and drop  */
/* *********************************************************************************************************** */
gboolean gtk_list_button_release_event ( GtkWidget *button,
					 GdkEventButton *ev )
{
    if ( !dragging_button )
	return FALSE;

    if ( ev )
	gdk_pointer_ungrab ( ev -> time );
    else
	gdk_pointer_ungrab ( GDK_CURRENT_TIME );

    g_signal_handler_disconnect ( G_OBJECT ( dragging_button ),
				  id_signal_drag_and_drop );


    /*     si on n'a pas bougé le bouton, c'est que c'est un click */

    if ( moved )
	g_signal_emit_by_name ( G_OBJECT ( dragging_button ),
				"reordered" );
    else
	g_signal_emit_by_name ( G_OBJECT ( dragging_button ),
				"clicked" );

    dragging_button = NULL;

    return FALSE;
}
/* *********************************************************************************************************** */




/* *********************************************************************************************************** */
/* ces fonction est appelée pendant un drag and drop si la souris sort */
/* du button en cours de drag */
/* *********************************************************************************************************** */
gint search_child_in_box ( GtkBoxChild *box_child,
			   GtkWidget *child )
{
    return ( !(child == box_child -> widget ));
}
gboolean leaving_button_while_dragging ( GtkWidget *button,
					 GdkEventCrossing *ev )
{
    gint x, y;
    gint position;
    gint longueur_liste;
    gint x_widget,y_widget;
    GtkWidget *dragging_button_init;
    GdkWindow *dragging_window;
    GtkWidget *dragging_parent;
    gint height_dragging_button;


    /*     si dragging_button est NULL, on ne devrait pas être ici... */

    if ( !dragging_button )
	return FALSE;


    gtk_widget_get_pointer ( dragging_button,
			     &x,
			     &y );

    /* si on est dans le widget, on fait rien */

    if ( y > 0
	 &&
	 y < dragging_button -> allocation.height )
	return FALSE;

    /*     on met moved à 1 pour éviter que le grab soit pris pour un click */

    moved = 1;

    /*     on récupère la fenetre ici car si l'event fin press button est réalisé, */
    /*     dragging_button devient null et plantage... */

    dragging_button_init = dragging_button;
    dragging_window = dragging_button -> window;
    dragging_parent = dragging_button -> parent;
    height_dragging_button = dragging_button -> allocation.height;

    /* on récupère maintenant les coordonnées du pointeur dans */
    /*     la vbox (le pointeur est bloqué dans cette window )*/

    gdk_window_get_pointer ( dragging_window,
			     &x,
			     &y,
			     NULL );

    gdk_window_get_position ( ev -> window,
			      &x_widget,
			      &y_widget );

    /*     on récupère la place du dragging_button dans la vbox */
    /*     et le nb de boutons dans la vbox */

    position = g_list_position ( GTK_BOX ( dragging_parent ) -> children,
				 g_list_find_custom ( GTK_BOX ( dragging_parent ) -> children,
						      dragging_button,
						      (GCompareFunc) search_child_in_box ));
    longueur_liste = g_list_length ( GTK_BOX ( dragging_parent ) -> children );


    if ( position == -1 )
	/* 	juste une protection au cas où, mais ne devrait jamais arriver (comme les bugs...) */
	return FALSE;


    /*     on va déplacer le dragging_button pour qu'il atteigne le pointeur */

    if ( y < y_widget )
    {
	/* 	le pointeur est au dessus du dragging_button */

	while ( y < y_widget
		&&
		position
		&&
		dragging_button )
	{
	    gtk_box_reorder_child ( GTK_BOX ( dragging_parent ),
				    GTK_WIDGET (dragging_button_init),
				    --position );
	    update_ecran ();
	    gdk_window_get_position ( ev -> window,
				      &x_widget,
				      &y_widget );
	    gdk_window_get_pointer ( dragging_window,
				     &x,
				     &y,
				     NULL );
	}
    }
    else
    {
	/* 	le pointeur est en dessous du dragging_button */

	while ( y >= y_widget + height_dragging_button
		&&
		position < longueur_liste
		&&
		dragging_button )
	{
	    gtk_box_reorder_child ( GTK_BOX ( dragging_parent ),
				    GTK_WIDGET (dragging_button_init),
				    ++position );
	    update_ecran ();
	    gdk_window_get_position ( ev -> window,
				      &x_widget,
				      &y_widget );
	    gdk_window_get_pointer ( dragging_window,
				     &x,
				     &y,
				     NULL );
	}
    }				    

/*     si y > à y_widget + height_dragging_button à ce niveau, c'est que la souris */
/* 	est en dessous du dernier bouton dans la box du parent */
/* 	on fait perdre le grab sinon ne récupère pas le bouton release event */

    if ( y > y_widget + height_dragging_button )
	gtk_list_button_release_event ( button,
					NULL );


    return ( FALSE );
}
/* *********************************************************************************************************** */

				

/* *********************************************************************************************************** */
/* retourne la donnée donnée à la création du bouton */
/* *********************************************************************************************************** */
gpointer gtk_list_button_get_data ( GtkListButton *button )
{
  g_return_val_if_fail (button != NULL, GTK_RELIEF_NORMAL);
  g_return_val_if_fail (GTK_IS_LIST_BUTTON (button), GTK_RELIEF_NORMAL);


  return ( GTK_LIST_BUTTON ( button ) -> data );
}
/* *********************************************************************************************************** */

