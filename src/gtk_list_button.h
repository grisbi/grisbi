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

#ifndef __GTK_LIST_BUTTON__H__
#define __GTK_LIST_BUTTON__H__


#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
 
  
#define GTK_TYPE_LIST_BUTTON		   (gtk_list_button_get_type ())
#define GTK_LIST_BUTTON(obj) GTK_CHECK_CAST(obj, gtk_list_button_get_type(), GtkListButton )
#define GTK_LIST_BUTTON_CLASS(klass) GTK_CHECK_CLASS_CAST( klass, gtk_list_button_get_type(), GtkListButtonClass )
#define GTK_IS_LIST_BUTTON(obj) GTK_CHECK_TYPE ( obj, gtk_list_button_get_type() )
  
  
typedef struct _GtkListButton GtkListButton;
typedef struct _GtkListButtonClass GtkListButtonClass;


struct _GtkListButton
{
  GtkButton button;

  GtkWidget *hbox;
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *closed_icon;
  GtkWidget *open_icon;
  gint group;
  gpointer data;
};


struct _GtkListButtonClass
{
  GtkButtonClass parent_class;
  void (* reordered)  (GtkButton *button);
};


guint gtk_list_button_get_type ( void );

/* ************************************************************** */
/* crée et renvoie un nouveau list_button contenant le text */
/* group entre 0 et 15 */
/* can_drag 1 ou 0 */
/* ************************************************************** */

GtkWidget *gtk_list_button_new ( gchar * text,
				 int group,
				 int can_drag,
				 gpointer data );


/* ************************************************************** */
/* ferme l'icône du bouton donné en argument */
/* ************************************************************** */
void gtk_list_button_close ( GtkButton *button );


/* ************************************************************** */
/* change le nom du list_button donné en argument */
/* ************************************************************** */

void gtk_list_button_set_name ( GtkListButton *, gchar * );


/* ************************************************************** */
/* récupère la donnée du bouton donné en argument */
/* ************************************************************** */

gpointer gtk_list_button_get_data ( GtkListButton *button );
				    


gboolean gtk_list_button_clicked ( GtkButton *button );


#endif				/* __GTK_LIST_BUTTON_H__ */
