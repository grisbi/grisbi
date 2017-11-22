/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2009 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2010Pierre Biava (grisbi@pierre.biava.name)                  */
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

/* WARNING this is a copy of test_integration.c (from ige-mac-integration-0.9.5) */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef GTKOSXAPPLICATION

#include "include.h"

/*START_INCLUDE*/
#include "grisbi_osx.h"
#include "export.h"
#include "gsb_account.h"
#include "gsb_assistant_account.h"
#include "gsb_assistant_archive.h"
#include "gsb_assistant_archive_export.h"
#include "gsb_debug.h"
#include "gsb_file.h"
#include "gsb_transactions_list.h"
#include "file_obfuscate.h"
#include "file_obfuscate_qif.h"
#include "import.h"
#include "menu.h"
#include "parametres.h"
#include "structures.h"
#include "erreur.h"
/*END_INCLUDE*/

typedef struct {
    GtkWindow *window;
    GtkWidget *open_item;
    GtkWidget *edit_item;
    GtkWidget *view_item;
    GtkWidget *quit_item;
    GtkWidget *help_menu;
    GtkWidget *about_item;
    GtkWidget *preferences_item;
} MenuItems;

typedef struct
{
    gchar *label;
    gpointer item;
} MenuCBData;

typedef struct
{
  GtkosxApplication *app;
  GtkosxApplicationAttentionType type;
} AttentionRequest;


/*START_STATIC*/
static void menu_items_destroy ( MenuItems *items );
static MenuItems *menu_items_new ( void );
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/


static GQuark menu_items_quark = 0;

/**
 *
 *
 *
 *
 * */
static MenuItems *menu_items_new ( void )
{
    return g_slice_new0 ( MenuItems );
}


/**
 *
 *
 *
 *
 * */
static void menu_items_destroy ( MenuItems *items )
{
    g_slice_free ( MenuItems, items );
}


/**
 *
 *
 *
 *
 * */
void grisbi_osx_app_active_cb ( GtkosxApplication* app, gboolean* data )
{
    g_print("Application became %s\n", *data ? "active" : "inactive");
}


/**
 *
 *
 *
 *
 * */
GtkWidget *grisbi_osx_init_menus ( GtkWidget *window, GtkWidget *menubar )
{
    GtkWidget *sep;
    MenuItems *items;
/*    GtkUIManager *ui_manager; */
    GtkosxApplication *theApp;

    theApp = g_object_new ( GTKOSX_TYPE_APPLICATION, NULL );

/*    ui_manager = gsb_menu_get_ui_manager ( );
*/    items = menu_items_new ( );

/*    items->open_item = gtk_ui_manager_get_widget ( ui_manager, "/menubar/FileMenu/Open" );
    items->edit_item = gtk_ui_manager_get_widget ( ui_manager, "/menubar/EditMenu" );
    items->view_item = gtk_ui_manager_get_widget ( ui_manager, "/menubar/ViewMenu" );
    items->help_menu = gtk_ui_manager_get_widget ( ui_manager, "/menubar/Help/Manual" );
    items->quit_item = gtk_ui_manager_get_widget ( ui_manager, "/menubar/FileMenu/Quit" );
    items->about_item = gtk_ui_manager_get_widget ( ui_manager, "/menubar/Help/About" );
    items->preferences_item = gtk_ui_manager_get_widget ( ui_manager, "/menubar/EditMenu/Preferences" );

*/    gtk_widget_set_sensitive ( items->preferences_item, FALSE );
    gtk_widget_hide ( items->quit_item );
    gtk_widget_hide ( menubar );

    gtkosx_application_set_menu_bar ( theApp, GTK_MENU_SHELL ( menubar ) );
    gtkosx_application_insert_app_menu_item  ( theApp, items->about_item, 0 );

    sep = gtk_separator_menu_item_new ( );
    g_object_ref ( sep );
    gtkosx_application_insert_app_menu_item  ( theApp, sep, 1 );
    gtkosx_application_insert_app_menu_item  ( theApp, items->preferences_item, 2);

    sep = gtk_separator_menu_item_new ( );
    g_object_ref ( sep );
    gtkosx_application_insert_app_menu_item  ( theApp, sep, 3 );

    gtkosx_application_set_help_menu ( theApp, GTK_MENU_ITEM ( items->help_menu ) );
    gtkosx_application_set_window_menu ( theApp, NULL );

    if ( !menu_items_quark )
        menu_items_quark = g_quark_from_static_string ( "MenuItem" );
    g_object_set_qdata_full ( G_OBJECT ( window ), menu_items_quark, items,
                        (GDestroyNotify ) menu_items_destroy );

    return menubar;
}


/**
 *
 *
 *
 *
 * */
void grisbi_osx_app_update_menus_cb ( void )
{
    GtkosxApplication *theApp;

    theApp = g_object_new ( GTKOSX_TYPE_APPLICATION, NULL );

    gtkosx_application_sync_menubar ( theApp );
}


#endif /* GTKOSXAPPLICATION */
/**
 *
 *
 *
 *
 * */
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
