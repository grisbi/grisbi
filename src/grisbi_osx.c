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

#ifdef GTKOSXAPPLICATION

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "grisbi_osx.h"
#include "menu.h"
/*END_INCLUDE*/


/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
/*END_EXTERN*/

typedef struct {
    GtkWindow *window;
    GtkWidget *open_item;
    GtkWidget *edit_item;
    GtkWidget *copy_item;
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
  GtkOSXApplication *app;
  GtkOSXApplicationAttentionType type;
} AttentionRequest;

static GQuark menu_items_quark = 0;

static GtkActionEntry test_actions[] =
{
    /*{Name, stock_id, label, accelerator, tooltip, callback} */
    {"FileMenuAction", NULL, "_File", NULL, NULL, NULL},
    {"OpenAction",  GTK_STOCK_OPEN, "_Open", NULL, NULL,
     G_CALLBACK(action_activate_cb)},
    {"QuitAction", GTK_STOCK_QUIT, "_Quit", NULL, NULL,
     G_CALLBACK(gtk_main_quit)},
    {"EditMenuAction", NULL, "_Edit", NULL, NULL, NULL },
    {"CopyAction", GTK_STOCK_COPY, "_Copy", NULL, NULL,
     G_CALLBACK(action_activate_cb)},
    {"PasteAction", GTK_STOCK_PASTE, "_Paste", NULL, NULL,
     G_CALLBACK(action_activate_cb)},
    {"PrefsAction", GTK_STOCK_PREFERENCES, "Pr_eferences", NULL, NULL, NULL },
    {"HelpMenuAction", NULL, "_Help", NULL, NULL, NULL },
    {"AboutAction", GTK_STOCK_ABOUT, "_About", NULL, NULL,
     G_CALLBACK(action_activate_cb)},
    {"HelpAction", GTK_STOCK_HELP, "_Help", NULL, NULL,
     G_CALLBACK(action_activate_cb)},
};

static GtkActionEntry view_menu[] =
{
    {"ViewMenuAction", NULL, "_View", NULL, NULL, NULL},
};

static GtkRadioActionEntry view_actions[] =
{
    /* Name, StockID, Label, Accelerator, Tooltip, Value */
    {"HorizontalAction", NULL, "_Horizontal", NULL, NULL, 0},
    {"VerticalAction", NULL, "_Vertical", NULL, NULL, 0},
};


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
static MenuCBData *menu_cbdata_new ( gchar *label, gpointer item )
{
    MenuCBData *datum =  g_slice_new0 ( MenuCBData );
    datum->label = label;
    datum->item = item;
    g_object_ref ( datum->item );

    return datum;
}


/**
 *
 *
 *
 *
 * */
static void menu_cbdata_delete ( MenuCBData *datum )
{
    g_object_unref ( datum->item );
    g_slice_free ( MenuCBData, datum );
}


/**
 *
 *
 *
 *
 * */
static void menu_item_activate_cb ( GtkWidget *item, MenuCBData  *datum )
{
    gboolean visible;
    gboolean sensitive;
    MenuItems *items = g_object_get_qdata ( G_OBJECT ( datum->item ), menu_items_quark );

    if ( GTK_IS_WINDOW ( G_OBJECT ( datum->item ) ) )
        g_print ( "Item activated: %s:%s\n",
                        gtk_window_get_title ( GTK_WINDOW ( datum->item ) ),
                        datum->label);
    else
        g_print ("Item activated %s\n", datum->label);

    if ( !items )
        return;

    g_object_get ( G_OBJECT ( items->copy_item ),
                        "visible", &visible,
                        "sensitive", &sensitive,
                        NULL);

    if ( item == items->open_item )
    {
        gtk_widget_set_sensitive ( items->copy_item, !gtk_widget_get_sensitive ( items->copy_item ) );
    }
}


/**
 *
 *
 *
 *
 * */
static void action_activate_cb ( GtkAction *action, gpointer data )
{
    GtkWindow *window = data;

    g_print ("Window %s, Action %s\n", gtk_window_get_title ( window ), gtk_action_get_name ( action ));
}


/**
 *
 *
 *
 *
 * */
static void radio_item_changed_cb ( GtkAction* action, GtkAction* current, MenuCBData *datum )
{
    g_print ("Radio group %s in window %s changed value: %s is now active.\n",
                        datum->label, gtk_window_get_title ( GTK_WINDOW ( datum->item ) ),
                        gtk_action_get_name ( GTK_ACTION ( current ) ) );
}


/**
 *
 *
 *
 *
 * */
static gboolean attention_cb ( AttentionRequest* req )
{
    gtk_osxapplication_attention_request ( req->app, req->type );
  
    g_free(req);

    return FALSE;
}


/**
 *
 *
 *
 *
 * */
static void bounce_cb ( GtkWidget  *button, GtkOSXApplication *app )
{
    AttentionRequest *req = g_new0 ( AttentionRequest, 1 );

    req->app = app;
    req->type = CRITICAL_REQUEST;
    g_timeout_add_seconds ( 2, (GSourceFunc) attention_cb, req );
    g_print("Now switch to some other application\n");
}


/**
 *
 *
 *
 *
 * */
static void change_icon_cb ( GtkWidget  *button, GtkOSXApplication *app )
{
    static gboolean   changed;
    static GdkPixbuf *pixbuf;

    if ( !pixbuf )
    {
        char filename [PATH_MAX];
        snprintf (filename, sizeof ( filename ), "%s/%s", PREFIX, "share/gtk-2.0/demo/gnome-foot.png");
        pixbuf = gdk_pixbuf_new_from_file ( filename, NULL );
    }

    if ( changed )
        gtk_osxapplication_set_dock_icon_pixbuf ( app, NULL );
    else
        gtk_osxapplication_set_dock_icon_pixbuf ( app, pixbuf );

    changed = !changed;
}


/**
 *
 *
 *
 *
 * */
static void change_menu_cb ( GtkWidget  *button, gpointer    user_data )
{
    GtkWidget *window = gtk_widget_get_toplevel ( button );
    MenuItems *items = g_object_get_qdata ( G_OBJECT ( window ), menu_items_quark );
    const gchar* open_accel_path = gtk_menu_item_get_accel_path ( GTK_MENU_ITEM ( items->open_item ) );
    const gchar* quit_accel_path = gtk_menu_item_get_accel_path ( GTK_MENU_ITEM (items->quit_item ) );

    if ( gtk_widget_get_visible ( items->edit_item ) )
    {
        gtk_widget_set_visible ( items->edit_item, FALSE );
        gtk_accel_map_change_entry ( open_accel_path, GDK_o, GDK_MOD1_MASK, TRUE );
        gtk_accel_map_change_entry(quit_accel_path, GDK_q, GDK_MOD1_MASK, TRUE );
    }
    else
    {
         gtk_widget_set_visible ( items->edit_item, TRUE );
         gtk_accel_map_change_entry ( open_accel_path, GDK_o, GDK_CONTROL_MASK, TRUE );
         gtk_accel_map_change_entry ( quit_accel_path, GDK_q, GDK_CONTROL_MASK, TRUE );
    }
}


/**
 *
 *
 *
 *
 * */
static void view_menu_cb (GtkWidget *button, gpointer user_data)
{
    GtkToggleButton *toggle = GTK_TOGGLE_BUTTON(button);
    static guint mergeid = 0;
    static GtkActionGroup* view_action_group = NULL;
    GtkUIManager *mgr = user_data;
    GtkWidget *window = gtk_widget_get_toplevel ( button );
    GtkOSXApplication *theApp = g_object_new ( GTK_TYPE_OSX_APPLICATION, NULL );
    GError *err = NULL;

    if ( view_action_group == NULL )
    {
        view_action_group = gtk_action_group_new ( "ViewAction" );
        gtk_action_group_add_actions ( view_action_group,
                        view_menu,
                        sizeof ( view_menu ) / sizeof ( GtkActionEntry ),
                        NULL );
        gtk_action_group_add_radio_actions_full ( view_action_group,
                        view_actions,
                        sizeof ( view_actions ) / sizeof ( GtkRadioActionEntry ),
                        0,
                        G_CALLBACK ( radio_item_changed_cb ),
                        menu_cbdata_new ( "View", GTK_WINDOW ( window ) ),
                        (GDestroyNotify) menu_cbdata_delete );
    }

    if ( gtk_toggle_button_get_active ( toggle ) )
    {
        mergeid = gtk_ui_manager_add_ui_from_file ( mgr, "src/addedui.xml", &err );
        if ( err )
        {
          g_print("Error retrieving file: %d %s\n", mergeid, err->message);
        }

        gtk_ui_manager_insert_action_group ( mgr, view_action_group, 0 );
        {
            GtkWidget *menu = gtk_menu_new ( );
            GtkWidget *item;

            item = gtk_menu_item_new_with_label ( "Framish" );
            g_signal_connect_data ( item,
                        "activate",
                        G_CALLBACK ( menu_item_activate_cb ),
                        menu_cbdata_new ( "Framish", item ),
                        (GClosureNotify) menu_cbdata_delete,
                        0 );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
            item = gtk_menu_item_new_with_label ( "Freebish" );
            g_signal_connect_data ( item,
                        "activate",
                        G_CALLBACK ( menu_item_activate_cb ),
                        menu_cbdata_new ( "Freebish", item ),
                        (GClosureNotify) menu_cbdata_delete,
                        0 );
            gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), item );
            gtk_osxapplication_set_dock_menu ( theApp, GTK_MENU_SHELL ( menu ) );
        }
    }
    else if ( mergeid )
    {
        gtk_ui_manager_remove_action_group ( mgr, view_action_group );
        gtk_ui_manager_remove_ui ( mgr, mergeid );
        mergeid = 0;
    }
}


/**
 *
 *
 *
 *
 * */
void grisbi_osx_app_active_cb ( GtkOSXApplication* app, gboolean* data )
{
    g_print("Application became %s\n", *data ? "active" : "inactive");
}


/**
 *
 *
 *
 *
 * */
gboolean grisbi_osx_app_should_quit_cb ( GtkOSXApplication *app, gpointer data )
{
    static gboolean abort = TRUE;
    gboolean answer;

    answer = abort;
    abort = FALSE;
    g_print ("Application has been requested to quit, %s\n", answer ? "but no!" : "it's OK.");

    return answer;
}


/**
 *
 *
 *
 *
 * */
void grisbi_osx_app_will_quit_cb ( GtkOSXApplication *app, gpointer data )
{
    g_print ("Quitting Now\n");

    gtk_main_quit();
}


/**
 *
 *
 *
 *
 * */
GtkWidget *grisbi_osx_create_window ( gchar *title )
{
    gpointer dock = NULL;
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *menubar;
    GtkWidget *bbox;
    GtkWidget *button;
    GtkWidget *sep;
    MenuItems *items = menu_items_new ();
    GtkUIManager *mgr = gtk_ui_manager_new ();
    GtkActionGroup *actions = gtk_action_group_new ("TestActions");
    guint mergeid;
    GError *err = NULL;

    GtkOSXApplication *theApp = g_object_new(GTK_TYPE_OSX_APPLICATION, NULL);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    if (title)
        gtk_window_set_title (GTK_WINDOW (window), title);
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 300);
    items->window = GTK_WINDOW (window);
    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    mergeid = gtk_ui_manager_add_ui_from_file ( mgr, "src/testui.xml", &err );
    if ( err )
    {
        g_print("Error retrieving file: %d %s\n", mergeid, err->message);
    }

    gtk_action_group_add_actions (actions,
                        test_actions,
                        sizeof ( test_actions ) / sizeof ( GtkActionEntry ),
                        (gpointer) window );

    gtk_ui_manager_insert_action_group ( mgr, actions, 0 );

    menubar = gtk_ui_manager_get_widget ( mgr, "/menubar" );
    items->open_item = gtk_ui_manager_get_widget ( mgr, "/menubar/File/Open" );
    items->edit_item = gtk_ui_manager_get_widget ( mgr, "/menubar/Edit" );
    items->copy_item = gtk_ui_manager_get_widget ( mgr, "/menubar/Edit/Copy" );
    items->help_menu = gtk_ui_manager_get_widget ( mgr, "/menubar/Help" );
    items->quit_item = gtk_ui_manager_get_widget ( mgr, "/menubar/File/Quit" );
    items->about_item = gtk_ui_manager_get_widget ( mgr, "/menubar/Help/About" );
    items->preferences_item = gtk_ui_manager_get_widget ( mgr, "/menubar/Edit/Preferences" );

    gtk_box_pack_start (GTK_BOX (vbox),  menubar, FALSE, TRUE, 0 );
  
    gtk_box_pack_start (GTK_BOX (vbox), gtk_label_new ("Some window content here"),  FALSE, FALSE, 12);

    bbox = gtk_hbutton_box_new ( );
    gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_CENTER);
    gtk_box_set_spacing (GTK_BOX (bbox), 12);

    gtk_box_pack_start (GTK_BOX (vbox),  bbox, TRUE, TRUE, 0);

    button = gtk_button_new_with_label ("Bounce");
    g_signal_connect (button, "clicked", G_CALLBACK ( bounce_cb ), dock );
    gtk_box_pack_start (GTK_BOX (bbox), button, FALSE, FALSE, 0 );

    button = gtk_button_new_with_label ("Change Icon");
    g_signal_connect (button, "clicked", G_CALLBACK (change_icon_cb), dock);
    gtk_box_pack_start (GTK_BOX (bbox), button, FALSE, FALSE, 0);

    button = gtk_button_new_with_label ("Change Menu");
    g_signal_connect (button, "clicked", G_CALLBACK (change_menu_cb), NULL);
    gtk_box_pack_start (GTK_BOX (bbox), button, FALSE, FALSE, 0);
    button = gtk_toggle_button_new_with_label("View Menu");

    g_signal_connect(button, "toggled", G_CALLBACK (view_menu_cb), (gpointer)mgr);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
    gtk_box_pack_start (GTK_BOX (bbox), button, FALSE, FALSE, 0);

    gtk_widget_show_all ( window );

    gtk_widget_hide ( menubar );

    gtk_osxapplication_set_menu_bar ( theApp, GTK_MENU_SHELL ( menubar ) );
    gtk_osxapplication_insert_app_menu_item  ( theApp, items->about_item, 0 );
  
    sep = gtk_separator_menu_item_new ( );
    g_object_ref ( sep );
    gtk_osxapplication_insert_app_menu_item  ( theApp, sep, 1 );
    gtk_osxapplication_insert_app_menu_item  ( theApp, items->preferences_item, 2);
  
    sep = gtk_separator_menu_item_new();
    g_object_ref(sep);
    gtk_osxapplication_insert_app_menu_item  ( theApp, sep, 3 );

    gtk_osxapplication_set_help_menu ( theApp, GTK_MENU_ITEM ( items->help_menu ) );
    gtk_osxapplication_set_window_menu ( theApp, NULL );

    if ( !menu_items_quark )
        menu_items_quark = g_quark_from_static_string ( "MenuItem" );
    g_object_set_qdata_full ( G_OBJECT ( window ), menu_items_quark, items, (GDestroyNotify )menu_items_destroy );

    return window;
}


void grisbi_osx_print_element ( void )
{
    return;
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
