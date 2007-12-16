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
 * \file gsb_assistant_account.c
 * we find here the complete assistant to create a new account
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_account.h"
#include "./gsb_assistant.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
/* TODO dOm : this function seems not to be used. Is it possible to remove it ?
static  GtkWidget *gsb_assistant_account_page_2 ( GtkWidget *assistant );
*/
/* TODO dOm : this function seems not to be used. Is it possible to remove it ?
static  GtkWidget *gsb_assistant_account_page_finish ( GtkWidget *assistant );
*/

/* TODO dOm : this function seems not to be used. Is it possible to remove it ?
static GtkResponseType gsb_assistant_account_run ( void );
*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

enum first_assistant_page
{
    ACCOUNT_ASSISTANT_INTRO= 0,
    ACCOUNT_ASSISTANT_PAGE_1,
    ACCOUNT_ASSISTANT_PAGE_FINISH,
};


/**
 * this function is called to launch the account assistant
 *
 * \param
 *
 * \return a GtkResponseType containing the return value at the end of the assistant
 * */
/* TODO dOm : this function seems not to be used. Is it possible to remove it ?
GtkResponseType gsb_assistant_account_run ( void )
{
    GtkResponseType return_value;
    GtkWidget *assistant;

    assistant = gsb_assistant_new ( _("Create a new account"),
				    _("Hello ! i'm the barbie girl who help you through that so hard way than creating a new account !"),
				    NULL,
				    NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_account_page_2 (assistant),
			     ACCOUNT_ASSISTANT_PAGE_1,
			     ACCOUNT_ASSISTANT_INTRO,
			     ACCOUNT_ASSISTANT_PAGE_FINISH,
			     NULL );
    gsb_assistant_add_page ( assistant,
			     gsb_assistant_account_page_finish (assistant),
			     ACCOUNT_ASSISTANT_PAGE_FINISH,
			     ACCOUNT_ASSISTANT_PAGE_1,
			     0,
			     NULL );

    return_value = gsb_assistant_run (assistant);
    gtk_widget_destroy (assistant);
    return return_value;
}
*/



/**
 * create the page 2 of the account assistant
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
/* TODO dOm : this function seems not to be used. Is it possible to remove it ?
static GtkWidget *gsb_assistant_account_page_2 ( GtkWidget *assistant )
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

    !* set up the menu *!
    label = gtk_label_new (_("This is the page 2... blah blah blah encore"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
    return page;
}
*/

/**
 * create the last page of the account assistant
 *
 * \param assistant the GtkWidget assistant
 *
 * \return a GtkWidget containing the page
 * */
/* TODO dOm : this function seems not to be used. Is it possible to remove it ?
static GtkWidget *gsb_assistant_account_page_finish ( GtkWidget *assistant )
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

    !* set up the menu *!
    label = gtk_label_new (_("Oh ! you succeed to create the account !!! another one to keep health ???\n"
			     "[button] oh yes\n"
			     "[button] no\n"));
    gtk_misc_set_alignment ( GTK_MISC (label),
			     0, 0.5 );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 label,
			 FALSE, FALSE, 0 );

    gtk_widget_show_all (page);
    return page;
}
*/


