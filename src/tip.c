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
#include "fichier_configuration_constants.h"
#include "dialog.h"
#include "utils_buttons.h"
#define END_INCLUDE

#define START_STATIC
gchar * get_next_tip ();
gchar * get_prev_tip ();
gchar * format_tip ( gchar * tip );
void change_button_sensitiveness ( GtkWidget * dialog, gint button, gboolean state );
gint max;
#define END_STATIC


void force_display_tip (  )
{
  display_tip ( TRUE );
}

void display_tip ( gboolean force )
{
  GtkWidget * checkbox;
  GtkWidget * dialog = NULL;
  gchar * tip;

  if ( !force && etat.show_tip )
    return;

  etat.last_tip ++;
  tip = get_next_tip ();

  dialog = dialogue_special_no_run ( GTK_MESSAGE_INFO, GTK_BUTTONS_NONE,
				     make_hint ( _("Did you know that..."),
						 _(tip) ) );

  checkbox = new_checkbox_with_title ( _("Do not show this message again"), 
				       &(etat.show_tip), NULL );
  gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(dialog) -> vbox ), checkbox, FALSE, FALSE, 6 );
  gtk_widget_show ( checkbox );

  gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
			   GTK_STOCK_GO_BACK, 1,
			   GTK_STOCK_GO_FORWARD, 2,
			   GTK_STOCK_CLOSE, 3,
			   NULL );
  
  while ( TRUE )
    {
      if ( max == etat.last_tip )
	change_button_sensitiveness ( dialog, 1, FALSE );
      if ( etat.last_tip == 1 )
	change_button_sensitiveness ( dialog, 0, FALSE );

      switch ( gtk_dialog_run ( GTK_DIALOG(dialog) ) )
	{
	case 1:
	  if ( etat.last_tip > 1 )
	    etat.last_tip --;
	  change_button_sensitiveness ( dialog, 1, TRUE ); 
	  gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog) -> label ), 
				 make_hint ( _("Did you know that..."),
					     _( get_next_tip () ) ) );
	  break;

	case 2:
	  etat.last_tip ++;
	  gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog) -> label ), 
				 make_hint ( _("Did you know that..."),
					     _( get_next_tip () ) ) );
	  change_button_sensitiveness ( dialog, 0, TRUE );
	  break;

	default:
	  gtk_widget_destroy ( dialog );
	  return;
	}
    }
}



gchar * get_next_tip ()
{
  gchar * buffer, * tip = NULL, ** tips;
  gint length;

  g_file_get_contents ( g_strconcat ( HELP_PATH, C_DIRECTORY_SEPARATOR, "tips.txt", NULL ),
			&buffer, &length, NULL );
  tips = g_strsplit ( buffer, "\"\n\n", length );

  for ( ; tips && *tips ; tips++ )
    {
      tip = *tips;
      if ( !*(tips+1) ||
	   (g_strtod ( (gchar *) tip, NULL ) == etat.last_tip ) )
	{
	  etat.last_tip = g_strtod ( (gchar *) tip, &tip );
	  if ( !*(tips+1) )
	    max = etat.last_tip;
	  break;
	}
    }

  printf ("> going to display %d\n", etat.last_tip);

  return format_tip ( tip );
}



gchar * format_tip ( gchar * tip )
{
  g_strstrip ( tip );
  if ( g_str_has_prefix ( tip, "- " ) )
    {
      tip += 2;
    }
  if ( g_str_has_prefix ( tip, "\"" ) )
    {
      tip ++;
    }
  while ( g_str_has_suffix ( tip, "\n" ) )
    {
      tip[strlen(tip)-1] = '\0';
    }
  if ( g_str_has_suffix ( tip, "\"" ) )
    {
      tip[strlen(tip)-1] = '\0';
    }

  return tip;
}



void change_button_sensitiveness ( GtkWidget * dialog, gint button, gboolean state )
{
  GSList * iter;

  if ( ! dialog )
    return;

  iter = gtk_container_get_children ( GTK_DIALOG(dialog) -> action_area);

  while ( iter )
    {
      if ( button == 2 )
	{
	  gtk_widget_set_sensitive ( GTK_WIDGET (iter -> data), state );
	}
      
      button ++;
      iter = iter -> next;
    }
}
