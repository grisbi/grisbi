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
 * \file gsb_assistant_archive.c
 * we find here the complete assistant to archive some transactions
 */


#include "include.h"

/*START_INCLUDE*/
#include "gsb_assistant_archive.h"
#include "./gsb_assistant.h"
#include "./gsb_assistant_reconcile_config.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

/*START_EXTERN*/
/*END_EXTERN*/

enum archive_assistant_page
{
    ARCHIVE_ASSISTANT_INTRO= 0,
    ARCHIVE_ASSISTANT_MENU,
    ARCHIVE_ASSISTANT_NEW_RECONCILE,
    ARCHIVE_ASSISTANT_AUTOMATICALY_ASSOCIATE,
    ARCHIVE_ASSISTANT_MANUALLY_ASSOCIATE,
    ARCHIVE_ASSISTANT_SUCCESS
};


/**
 * this function is called to launch the assistant to make archives
 *
 * \param
 *
 * \return a GtkResponseType containing the return value at the end of the assistant
 * */
GtkResponseType gsb_assistant_archive_run ( void )
{
    GtkResponseType return_value;
    GtkWidget *assistant;

    /* create the assistant */
    assistant = gsb_assistant_new ( _("Archive the transactions"),
				    _("This assistant will help you to file some transactions, to increase the speed of grisbi.\n"
				      "By default, Grisbi doesn't export an archive. It keeps the transactions into the file,\n"
				      "but don't work with them anymore.\n"
				      "Obviously, you will be able to show them again and work on them, and you will be able\n"
				      "to export them in an archive file if necessary.\n"),
				    "grisbi-logo.png" );
/*     gsb_assistant_add_page ( assistant, */
/* 			     gsb_assistant_reconcile_config_page_menu (assistant), */
/* 			     ARCHIVE_ASSISTANT_MENU, */
/* 			     ARCHIVE_ASSISTANT_INTRO, */
/* 			     ARCHIVE_ASSISTANT_NEW_RECONCILE, */
/* 			     NULL ); */
/*     gsb_assistant_add_page ( assistant, */
/* 			     gsb_assistant_reconcile_config_page_new_reconcile (), */
/* 			     ARCHIVE_ASSISTANT_NEW_RECONCILE, */
/* 			     ARCHIVE_ASSISTANT_MENU, */
/* 			     ARCHIVE_ASSISTANT_MENU, */
/* 			     NULL ); */
/*     gsb_assistant_add_page ( assistant, */
/* 			     gsb_assistant_reconcile_config_page_automaticaly_associate (assistant), */
/* 			     ARCHIVE_ASSISTANT_AUTOMATICALY_ASSOCIATE, */
/* 			     ARCHIVE_ASSISTANT_MENU, */
/* 			     ARCHIVE_ASSISTANT_MENU, */
/* 			     G_CALLBACK (gsb_assistant_reconcile_config_update_auto_asso)); */
/*     gsb_assistant_add_page ( assistant, */
/* 			     gsb_assistant_reconcile_config_page_manually_associate (assistant), */
/* 			     ARCHIVE_ASSISTANT_MANUALLY_ASSOCIATE, */
/* 			     ARCHIVE_ASSISTANT_MENU, */
/* 			     ARCHIVE_ASSISTANT_MENU, */
/* 			     G_CALLBACK (gsb_assistant_reconcile_config_update_manu_asso)); */
/*     gsb_assistant_add_page ( assistant, */
/* 			     gsb_assistant_reconcile_config_page_success (), */
/* 			     ARCHIVE_ASSISTANT_SUCCESS, */
/* 			     ARCHIVE_ASSISTANT_MENU, */
/* 			     ARCHIVE_ASSISTANT_MENU, */
/* 			     NULL ); */
    return_value = gsb_assistant_run (assistant);
    gtk_widget_destroy (assistant);
    return return_value;

    return FALSE;
}
