/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2008 Cédric Auger (cedric@grisbi.org)	      */
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

/**
 * \file print_dialog.c
 * dialog functions for printing
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "print_dialog_config.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *window;
/*END_EXTERN*/

static GtkPrintSettings *settings = NULL;

/**
 * general setup dialog to print
 *
 * \param begin_callback	the function to call when the "begin" signal is emited
 * \param draw_callback		the function to call when the "draw" signal is emited
 * \param custom_tab_label	Tab title for an optional custom config tab
 * \param custom_tab_callback	Callback that creates an optional config tab 
 * \param custom_apply_callback	Callback called when user validate config
 * \param data data for callback
 *
 */
void print_dialog_config ( GCallback begin_callback,
                        GCallback draw_callback,
                        gchar *custom_tab_label,
                        GCallback custom_tab_callback,
                        GCallback custom_apply_callback,
                        gpointer data )
{
    GtkPrintOperation *print;
    GtkPrintOperationResult res;

    print = gtk_print_operation_new ();
	gtk_print_operation_set_unit ( print, GTK_UNIT_POINTS );

    if (settings != NULL) 
	gtk_print_operation_set_print_settings (print, settings);

    /* Custom stuff */
    if ( custom_tab_label )
	gtk_print_operation_set_custom_tab_label ( print, custom_tab_label );
    if ( custom_tab_callback )
	g_signal_connect (print, "create-custom-widget", custom_tab_callback, data );
    if ( custom_apply_callback )
	g_signal_connect (print, "custom-widget-apply", custom_apply_callback, data );

    g_signal_connect (print, "begin_print", G_CALLBACK (begin_callback), data );
    g_signal_connect (print, "draw_page", G_CALLBACK (draw_callback), data );


    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				   GTK_WINDOW (window), NULL);

    if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
    {
	settings = gtk_print_operation_get_print_settings (print);
    }

}
