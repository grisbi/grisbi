/* ************************************************************************** */
/* Ce fichier s'occupe des astuces					      */
/*                                                                            */
/*     Copyright (C)	2004      Benjamin Drieu (bdrieu@april.org)	      */
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


#include "include.h"


#define START_INCLUDE
#include "dialog.h"
#define END_INCLUDE

#define START_STATIC
#define END_STATIC


void display_tip ()
{
  GtkWidget * dialog, *checkbox;;

  dialog = dialogue_special_no_run ( GTK_MESSAGE_INFO, GTK_BUTTONS_NONE,
				     make_hint (_("Did you know that..."),
						_("You can import QIF, OFX or Gnucash files into your accounts, so that you can switch to Grisbi without loosing your accounts?")));

  checkbox = new_checkbox_with_title ( _("Do not show this message again"), NULL, NULL );
  gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(dialog) -> vbox ), checkbox, TRUE, TRUE, 6 );
  gtk_widget_show ( checkbox );

  gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
			   GTK_STOCK_GO_BACK, 1,
			   GTK_STOCK_GO_FORWARD, 2,
			   GTK_STOCK_CLOSE, 3,
			   NULL );
  
  gtk_dialog_run ( GTK_DIALOG(dialog) );
  gtk_widget_destroy ( dialog );
}
