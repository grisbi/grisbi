/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2007 Benjamin Drieu (bdrieu@april.org)	      */
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
 * \file gsb_assistant_first.c
 * we find here the complete assistant launched at the first use of grisbi
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_first.h"
#include "./gsb_assistant.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  GtkWidget *gsb_assistant_first_page_2 ( GtkWidget *assistant );
static  GtkWidget *gsb_assistant_first_page_finish ( GtkWidget *assistant );
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

enum first_assistant_page
{
    FIRST_ASSISTANT_INTRO= 0,
    FIRST_ASSISTANT_PAGE_1,
    FIRST_ASSISTANT_PAGE_FINISH,
};


/**
 * this function is called to launch the first opening assistant
 *
 * \param
 *
 * \return a GtkResponseType containing the return value at the end of the assistant
 * */
GtkResponseType gsb_assistant_first_run ( void )
{
    GtkResponseType return_value;
    GtkWidget *assistant;

    /* create the assistant */
    assistant = gsb_assistant_new ( _("Welcome to Grisbi !"),
				    _("Blah, blah, blah... euh... blahblah"),
				    NULL,
				    NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_2 (assistant),
			     FIRST_ASSISTANT_PAGE_1,
			     FIRST_ASSISTANT_INTRO,
			     FIRST_ASSISTANT_PAGE_FINISH,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_first_page_finish (assistant),
			     FIRST_ASSISTANT_PAGE_FINISH,
			     FIRST_ASSISTANT_PAGE_1,
			     0,
			     NULL );

    return_value = gsb_assistant_run (assistant);
    gtk_widget_destroy (assistant);
    return return_value;
}



/**
 * create the page 2 of the first assistant
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_first_page_2 ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* set up the menu */
    label = gtk_label_new (_("This is the page 2... blah blah blah encore"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
    return page;
}

/**
 * create the last page of the first assistant
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
static GtkWidget *gsb_assistant_first_page_finish ( GtkWidget *assistant )
{
    GtkWidget *page;
    GtkWidget *vbox;
    GtkWidget *label;

    page = gtk_hbox_new (FALSE, 15);
    gtk_container_set_border_width ( GTK_CONTAINER (page),
				     10 );

    vbox = gtk_vbox_new (FALSE, 5);
    gtk_box_pack_start ( GTK_BOX (page),
			 vbox,
			 FALSE, FALSE, 0 );

    /* set up the menu */
    label = gtk_label_new (_("This is the last page... congratulation, grisbi is configured\n"
			     "do you want to use the assistant to create a new account ?\n"
			     "[button] oh yes ! i want to use the assistant\n"
			     "[button] no, i'm a rough guy, i will find by myself !\n"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
    return page;
}


