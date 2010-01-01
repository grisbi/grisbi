/* ************************************************************************** */
/* Ce fichier s'occupe des astuces                                            */
/*                                                                            */
/*     Copyright (C)    2004-2008 Benjamin Drieu (bdrieu@april.org)           */
/*          2009 Pierre Biava (grisbi@pierre.biava.name)                      */
/*          http://www.grisbi.org                                             */
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
#include "tip.h"
#include "./dialog.h"
#include "./gsb_automem.h"
#include "./utils_str.h"
#include "./include.h"
#include "./structures.h"
#define END_INCLUDE

#define START_STATIC
static void change_button_sensitiveness ( GtkWidget * dialog, gint button, gboolean state );
static gchar * format_tip ( gchar * tip );
static gchar * get_next_tip ();
#define END_STATIC

gint max;

/** 
 * Display a tip forcefully, even if show_tip option has been disabled.
 */
void force_display_tip (  )
{
    display_tip ( TRUE );
}



/**
 * Display a tip.
 *
 * \param force  Forcefully display the tip even if show_tip option
 *		 has been disabled.
 */
void display_tip ( gboolean force )
{
    GtkWidget * checkbox;
    GtkWidget * dialog = NULL;
    gchar * tip;
    gchar *tmpstr;

    if ( !force && !etat.show_tip )
        return;

    etat.last_tip ++;
    tip = get_next_tip ();

    // If no tips found ... no dialog will be displayed...
    if (!tip)
    {
        dialog_message ( "no-tip-available" );
        return;
    }
    dialog = dialogue_special_no_run ( GTK_MESSAGE_INFO, GTK_BUTTONS_NONE,
                        make_hint ( _("Did you know that..."),
                        /* We use the grisbi-tips catalog */
                        dgettext("grisbi-tips", (tip) ) ) );
    gtk_window_set_modal ( GTK_WINDOW ( dialog ), FALSE );

    checkbox = gsb_automem_checkbutton_new ( _("Display tips at next start"), 
                        &(etat.show_tip), NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( GTK_DIALOG(dialog) -> vbox ), checkbox, FALSE, FALSE, 6 );
    gtk_widget_show ( checkbox );

    gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
                        GTK_STOCK_GO_BACK, 1,
                        GTK_STOCK_GO_FORWARD, 2,
                        GTK_STOCK_CLOSE, 3,
                        NULL );
 
    gtk_widget_set_size_request ( dialog, 450, -1 );
    /* We iterate as user can select several tips. */
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
        tmpstr = g_strconcat ( make_pango_attribut (
                        "size=\"larger\" weight=\"bold\"", _("Did you know that...") ),
                        "\n\n",
                        dgettext ("grisbi-tips", get_next_tip ( ) ),
                        NULL );

        gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog) -> label ),
                        tmpstr );
        g_free ( tmpstr );
        break;

        case 2:
        etat.last_tip ++;
        tmpstr = g_strconcat ( make_pango_attribut (
                        "size=\"larger\" weight=\"bold\"", _("Did you know that...") ),
                        "\n\n",
                        dgettext ("grisbi-tips", get_next_tip ( ) ),
                        NULL );

        gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog) -> label ),
                        tmpstr );
        g_free ( tmpstr );
        change_button_sensitiveness ( dialog, 0, TRUE );
        break;

        default:
        gtk_widget_destroy ( dialog );
        return;
    }
    }
}


/**
 * Returns the string representation of next tip to be displayed,
 * according to last tip displayed.
 *
 * \return Pango-formated string of the tip.
 */
gchar * get_next_tip ()
{
  gchar * buffer, * tip = NULL, ** tips;
  gsize length;
  gchar *filename;

  // If there any problem during tip file reading, return NULL 
  filename = g_build_filename ( DATA_PATH, "tips.txt", 
				NULL );
  if ( ! g_file_get_contents ( filename,
			       &buffer, &length, NULL ) )
  {
      g_free (filename);
      return NULL;
  }
  g_free (filename);

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

  return format_tip ( tip );
}



/**
 * Remove any leading and trailing tokens that are included in the tip
 * file.
 *
 * \param tip	An unformated string read directly from the "tip.txt"
 *		file.
 *
 * \return	A string cleaned from any format tokens.
 */
gchar * format_tip ( gchar * tip )
{
    gchar * new, * tmp;

    while ( tip && ( isdigit ( *tip ) || isspace ( *tip ) ) )
    {
	tip++;
    }

    /* leading '- ' if any */
    if ( g_str_has_prefix ( tip, "_ " ) )
    {
	tip += 2;
    }

    /* leading '"' if any */
    if ( g_str_has_prefix ( tip, "\"" ) )
    {
	tip ++;
    }

    /* any '\n' at the end of the string */
    while ( g_str_has_suffix ( tip, "\n" ) )
    {
	tip[strlen(tip)-1] = '\0';
    }

    /* trailing '"' if any */
    if ( g_str_has_suffix ( tip, "\"" ) )
    {
	tip[strlen(tip)-1] = '\0';
    }

    new = my_strdup ( tip );
    for ( tmp = new ; * tmp ; tmp ++, tip ++ )
    {
	if ( * tip == '\\' )
	{
	    tip ++;
	}
	*tmp = *tip;
    }
    *tmp = '\0';    

    return new;
}



/**
 * Change sensitiveness of a one of the buttons of the dialog widget.
 *
 * \param dialog	Dialog widget that contains buttons.
 * \param button	Number of the button to change.
 * \param state		Sensitiveness to apply to the button.
 */
void change_button_sensitiveness ( GtkWidget * dialog, gint button, gboolean state )
{
  GSList * iter;

  if ( ! dialog )
    return;

  iter = (GSList *) gtk_container_get_children ( GTK_CONTAINER ( GTK_DIALOG(dialog) -> action_area) );

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

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
