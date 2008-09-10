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


#include "include.h"

/*START_INCLUDE*/
#include "print_dialog_config.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *window ;
/*END_EXTERN*/

#if GTK_CHECK_VERSION(2,10,0)

static GtkPrintSettings *settings = NULL;

/**
 * general setup dialog to print
 *
 * \param begin_callback	the function to call when the "begin" signal is emited
 * \param draw_callback		the function to call when the "draw" signal is emited
 *
 * \return
 * */
void print_dialog_config ( GCallback begin_callback,
			   GCallback draw_callback )
{
    GtkPrintOperation *print;
    GtkPrintOperationResult res;

    print = gtk_print_operation_new ();

    if (settings != NULL) 
	gtk_print_operation_set_print_settings (print, settings);

    g_signal_connect (print, "begin_print", G_CALLBACK (begin_callback), NULL);
    g_signal_connect (print, "draw_page", G_CALLBACK (draw_callback), NULL);


    res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				   GTK_WINDOW (window), NULL);

    if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
    {
	settings = gtk_print_operation_get_print_settings (print);
    }

}

#endif /* GTK_CHECK_VERSION(2,10,0) */
