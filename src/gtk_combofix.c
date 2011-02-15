/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2001-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2003-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          2009-2010 Pierre Biava (grisbi@pierre.biava.name)                 */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

#include <gdk/gdkkeysyms.h>
#include <ctype.h>

/*START_INCLUDE*/
#include "gtk_combofix.h"
/*END_INCLUDE*/

 
G_DEFINE_TYPE ( GtkComboFix, gtk_combofix, GTK_TYPE_HBOX );

#define GTK_COMBOFIX_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_COMBOFIX, GtkComboFixPrivate))

struct _GtkComboFixPrivate
{
    GtkWidget *button;
    GtkWidget *popup;
    GtkWidget *tree_view;

    GtkTreeStore *store;
    GtkTreeModel *model_filter;
    GtkTreeModel *model_sort;
    GtkTreeSelection *selection;

    /* TRUE for case sensitive (in that case, the first entry give the case) */
    gboolean case_sensitive;
    /* TRUE if the entry content must belong to the list  */
    gboolean force;
    /* TRUE mix the different list, FALSE separate them */
    gboolean mixed_sort;
    /* 0 to show all the items */
    gint max_items;
    /* number of items */
    gint visible_items;

    /* old entry */
    gchar *old_entry;
};


/* Liste des fonctions statiques */

/* common */
static void gtk_combofix_class_init ( GtkComboFixClass *klass );
static void gtk_combofix_dispose ( GObject *combofix );
static void gtk_combofix_finalize ( GObject *combofix );
static void gtk_combofix_init ( GtkComboFix *combofix );
static void gtk_combofix_dialog ( gchar *text, gchar *hint );

/* entry */
static gboolean gtk_combofix_entry_insert ( GtkComboFix *combofix );
static gboolean gtk_combofix_entry_delete ( GtkComboFix *combofix );
static gboolean gtk_combofix_entry_changed ( GtkComboFix *combofix,
                        gboolean insert_text );

/* popup */
static gboolean gtk_combofix_expose_entry ( GtkComboFix *combofix );
static gchar *gtk_combofix_update_visible_rows ( GtkComboFix *combofix,
                        const gchar *string );
static gboolean gtk_combofix_set_all_visible_rows ( GtkComboFix *combofix );
static gboolean gtk_combofix_set_popup_position ( GtkComboFix *combofix );
static gboolean gtk_combofix_button_release_event ( GtkWidget *popup,
                        GdkEventKey *ev,
                        GtkComboFix *combofix );
static gboolean gtk_combofix_button_press ( GtkWidget *popup,
                        GdkEventButton *ev,
                        GtkComboFix *combofix );
static gboolean gtk_combofix_focus_in ( GtkWidget *entry,
                        GdkEvent *ev,
                        GtkComboFix *combofix );
static gboolean  gtk_combofix_focus_out ( GtkWidget *entry,
                        GdkEvent *ev,
                        GtkComboFix *combofix );
static gboolean gtk_combofix_key_press_event ( GtkWidget *entry,
                        GdkEventKey *ev,
                        GtkComboFix *combofix );
static gboolean gtk_combofix_button_press_event ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        GtkComboFix *combofix );
static gboolean gtk_combofix_move_selection ( GtkComboFix *combofix,
                        gint direction );
static gint gtk_combofix_get_rows_number_by_page ( GtkComboFix *combofix );
static gboolean gtk_combofix_move_selection_one_step ( GtkComboFix *combofix,
                        GtkTreeIter *iter,
                        gint direction );
static gboolean gtk_combofix_separator_func ( GtkTreeModel *model,
                        GtkTreeIter *iter,
                        GtkComboFix *combofix );
static gboolean gtk_combofix_popup_key_press_event ( GtkWidget *popup,
                        GdkEventKey *ev,
                        GtkComboFix *combofix );

/* model */
static gboolean gtk_combofix_choose_selection ( GtkComboFix *combofix );
static gint gtk_combofix_default_sort_func ( GtkTreeModel *model_sort,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        GtkComboFix *combofix );
static gboolean gtk_combofix_fill_store ( GtkComboFix *combofix,
                        GSList *list,
                        gint list_number );
static gboolean gtk_combofix_fill_iter_child ( GtkTreeStore *store,
                        GtkTreeIter *iter_parent,
                        const gchar *string,
                        const gchar *real_string,
                        gint list_number );
static gboolean gtk_combofix_fill_iter_parent ( GtkTreeStore *store,
                        GtkTreeIter *iter_parent,
                        const gchar *string,
                        gint list_number );
static gchar *gtk_combofix_get_first_string_from_sort_model ( GtkTreeModel *sort_model,
                        const gchar *string );
static gboolean gtk_combofix_search_for_text (GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer *data);
static gboolean gtk_combofix_select_item ( GtkComboFix *combofix,
                        const gchar *item );

/* globals variables */
static gint block_expose_event;

enum combofix_columns {
    COMBOFIX_COL_VISIBLE_STRING = 0,    /* string : what we see in the combofix */
    COMBOFIX_COL_REAL_STRING,           /* string : what we set in the entry when selecting something */
    COMBOFIX_COL_VISIBLE,               /* boolean : if that line has to be showed */
    COMBOFIX_COL_LIST_NUMBER,           /* int : the number of the list 1, 2 ou 3 (CREDIT DEBIT SPECIAL) */
    COMBOFIX_COL_SEPARATOR,             /* TRUE : if this is a separator */
    COMBOFIX_N_COLUMNS,
};

enum combofix_key_direction {
    COMBOFIX_UP = 0,
    COMBOFIX_PAGE_UP,
    COMBOFIX_DOWN,
    COMBOFIX_PAGE_DOWN,
};

/*START_EXTERN*/
/*END_EXTERN*/


/* *********************** the first part contains all the extern functions ******************************************** */

/**
 * create a combofix, with several list set one after the others
 * by default, force is not set, no max items
 * and case unsensitive
 *
 * \param list a g_slist of name (\t at the beginning makes it as a child)
 * \param force TRUE and the text must be in the list
 * \param sort TRUE and the list will be sorted automatickly
 * \param max_items the minimum of characters to show the popup
 *
 * \return the new widget
 * */
GtkWidget *gtk_combofix_new ( GSList *list )
{
    GtkComboFix *combofix = g_object_new ( GTK_TYPE_COMBOFIX, NULL );
 
    gtk_combofix_set_list (combofix, list);

    return ( GTK_WIDGET ( combofix ) );
}


/**
 * get the text in the combofix
 *
 * \param combofix
 *
 * \return a const gchar
 * */
const gchar *gtk_combofix_get_text ( GtkComboFix *combofix )
{
    g_return_val_if_fail (combofix , NULL);
    g_return_val_if_fail (GTK_IS_COMBOFIX (combofix), NULL);

    return ( gtk_entry_get_text ( GTK_ENTRY ( combofix -> entry ) ) );
}


/**
 * set the text in the combofix without showing the popup or
 * doing any check
 *
 * \param combofix
 * \param text
 *
 * \return
 * */
void gtk_combofix_set_text ( GtkComboFix *combofix, const gchar *text )
{
    g_return_if_fail ( combofix );
    g_return_if_fail ( GTK_IS_COMBOFIX ( combofix ) );

    g_signal_handlers_block_by_func ( G_OBJECT ( combofix -> entry ),
                        G_CALLBACK ( gtk_combofix_entry_insert ),
                        combofix );
    g_signal_handlers_block_by_func ( G_OBJECT (combofix -> entry ),
                        G_CALLBACK ( gtk_combofix_entry_delete ),
                        combofix );

    if ( text && strlen ( text ) > 0 )
        gtk_entry_set_text ( GTK_ENTRY ( combofix -> entry ), text );
    else
        gtk_entry_set_text ( GTK_ENTRY ( combofix -> entry ), "" );

    g_signal_handlers_unblock_by_func ( G_OBJECT ( combofix -> entry ),
                        G_CALLBACK ( gtk_combofix_entry_insert ),
                        combofix );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( combofix -> entry ),
                        G_CALLBACK ( gtk_combofix_entry_delete ),
                        combofix );
}


/**
 * set the flag to force/unforce the text in the entry
 * if force is set, the value in the entry must belong to the list
 *
 * \param combofix
 * \param value
 *
 * \return
 * */
void gtk_combofix_set_force_text ( GtkComboFix *combofix, gboolean value )
{
    GtkComboFixPrivate *priv = combofix -> priv;

    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    priv->force = value;
}


/**
 * set if the completion is case sensitive or not
 *
 * \param combofix
 * \param case_sensitive TRUE or FALSE
 *
 * \return
 * */
void gtk_combofix_set_case_sensitive ( GtkComboFix *combofix, gboolean case_sensitive )
{
    GtkComboFixPrivate *priv = combofix -> priv;

    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    priv -> case_sensitive = case_sensitive;
}


/**
 * hide the popup
 *
 * \param combofix
 *
 * \return FALSE
 * */
gboolean gtk_combofix_hide_popup ( GtkComboFix *combofix )
{
    GtkComboFixPrivate *priv = combofix -> priv;

    g_return_val_if_fail ( combofix != NULL, FALSE );
    g_return_val_if_fail ( GTK_IS_COMBOFIX ( combofix ), FALSE );

    if ( GTK_WIDGET_VISIBLE ( priv -> popup ))
    {
        gtk_grab_remove ( priv -> popup );
        gdk_pointer_ungrab ( GDK_CURRENT_TIME );
        gtk_widget_hide ( priv -> popup );
    }
    return FALSE;
}


/**
 * show the popup with all the content, not according to the entry
 *
 * \param combofix
 *
 * return FALSE
 * */
gboolean gtk_combofix_show_popup ( GtkComboFix *combofix )
{
    GtkComboFixPrivate *priv = combofix -> priv;

    if ( GTK_WIDGET_VISIBLE ( priv -> popup ) )
        return FALSE;

    g_signal_handlers_block_by_func ( G_OBJECT ( combofix -> entry ),
                        G_CALLBACK ( gtk_combofix_entry_insert ),
                        combofix );
    g_signal_handlers_block_by_func ( G_OBJECT ( combofix -> entry ),
                        G_CALLBACK ( gtk_combofix_entry_delete ),
                        combofix );

    gtk_combofix_set_all_visible_rows ( combofix );
    gtk_combofix_set_popup_position ( combofix );
    gtk_widget_show ( priv -> popup );
    gtk_combofix_select_item ( combofix, gtk_combofix_get_text ( combofix ) );
    gtk_widget_grab_focus ( GTK_WIDGET ( combofix -> entry ));
    gtk_window_set_modal (GTK_WINDOW ( priv -> popup ), TRUE);

    g_signal_handlers_unblock_by_func ( G_OBJECT ( combofix -> entry),
                        G_CALLBACK ( gtk_combofix_entry_insert ),
                        combofix );
    g_signal_handlers_unblock_by_func ( G_OBJECT ( combofix -> entry ),
                        G_CALLBACK ( gtk_combofix_entry_delete ),
                        combofix );

    return FALSE;
}


/**
 * set the maximum items viewable in the popup,
 * if there is more items corresponding to the entry than that number,
 * the popup is not showed
 *
 * \param combofix
 * \param max_items
 *
 * \return
 * */
void gtk_combofix_set_max_items ( GtkComboFix *combofix, gint max_items )
{
    GtkComboFixPrivate *priv = combofix -> priv;

    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    priv -> max_items = max_items;
}


/**
 * set for the complex combofix if the different list have to
 * be mixed or separate
 *
 * \param combofix
 * \param mixed_sort TRUE or FALSE
 *
 * \return
 * */
void gtk_combofix_set_mixed_sort ( GtkComboFix *combofix, gboolean mixed_sort )
{
    GtkComboFixPrivate *priv = combofix -> priv;

    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    priv -> mixed_sort = mixed_sort;
}


/**
 * change the list of an existing combofix
 *
 * \param combofix
 * \param list the new list
 *
 * \return TRUE if ok, FALSE if problem
 * */
gboolean gtk_combofix_set_list ( GtkComboFix *combofix, GSList *list )
{
    GtkComboFixPrivate *priv = combofix -> priv;

    GSList *tmp_list;
    gint list_number = 0;
    gint length;
    GtkTreeIter iter;

    g_return_val_if_fail (combofix, FALSE );
    g_return_val_if_fail (GTK_IS_COMBOFIX (combofix), FALSE);
    g_return_val_if_fail (list, FALSE);

    gtk_tree_store_clear ( priv -> store );

    tmp_list = list;
    length = g_slist_length (list);

    while ( tmp_list )
    {
        gtk_combofix_fill_store ( combofix, tmp_list -> data, list_number );

        /* set the separator */
        if (list_number < ( length-1 ) )
        {
            gtk_tree_store_append ( priv -> store, &iter, NULL );
            gtk_tree_store_set ( priv -> store,
                        &iter,
                        COMBOFIX_COL_LIST_NUMBER, list_number,
                        COMBOFIX_COL_SEPARATOR, TRUE,
                        -1 );
        }
        list_number++;

        tmp_list = tmp_list -> next;
    }

    return TRUE;
}


/**
* append a new line in a combofix
*
* \param combofix text
*
* \return
* */
void gtk_combofix_append_text ( GtkComboFix *combofix, const gchar *text )
{
    GtkComboFixPrivate *priv;
    gchar **tab_char;
    gint empty;
    gpointer pointeurs[3] = { ( gpointer ) text, NULL, NULL };

    g_return_if_fail ( combofix );
    g_return_if_fail ( GTK_IS_COMBOFIX ( combofix ) );

    /* g_print ("gtk_combofix_append_text = %s\n", text ); */
    priv = combofix -> priv;
    pointeurs[2] = GINT_TO_POINTER ( priv -> case_sensitive );

    empty = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( combofix -> entry ), "empty" ) );
    if ( empty || priv -> force )
        return;

    if ( priv -> old_entry && strcmp ( text, priv -> old_entry ) == 0 )
        return;

    gtk_tree_model_foreach ( GTK_TREE_MODEL ( priv -> store ),
                        (GtkTreeModelForeachFunc) gtk_combofix_search_for_text,
                        pointeurs );

    if ( pointeurs[1] && GINT_TO_POINTER ( pointeurs[1] ) )
        return;

    tab_char = g_strsplit ( text, " : ", 2 );
    if (tab_char[0])
    {
        GtkTreeIter iter_parent;

        gtk_combofix_fill_iter_parent ( priv -> store, &iter_parent, text, 0 );

        if ( tab_char[1] )
        {
            gchar* tmpstr;

            tmpstr = g_strconcat ( "\t", text, NULL );
            gtk_combofix_fill_iter_child ( priv -> store, &iter_parent, tab_char[1], text, 0 );

            g_free ( tmpstr );
        }
    }

    if ( priv -> old_entry && strlen ( priv -> old_entry ) )
        g_free ( priv -> old_entry );
    priv -> old_entry = g_strdup ( text );
}


/**
* remove a line in a combofix
*
* \param combofix text
*
* \return
* */
void gtk_combofix_remove_text ( GtkComboFix *combofix, const gchar *text )
{
    GtkComboFixPrivate *priv = combofix -> priv;
    GtkTreeIter iter;
    gboolean case_sensitive = priv -> case_sensitive;
    gboolean valid;

    valid = gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( priv -> store ), &iter);

    while ( valid )
    {
        gchar *tmp_str;
        gboolean separator;

        gtk_tree_model_get ( GTK_TREE_MODEL ( priv -> store ), &iter,
                    COMBOFIX_COL_REAL_STRING, &tmp_str,
                    COMBOFIX_COL_SEPARATOR, &separator,
                    -1 );

        if ( separator )
        {
            valid = gtk_tree_model_iter_next ( GTK_TREE_MODEL ( priv -> store ), &iter );
            continue;
        }
        if ( case_sensitive && !strcmp ( text, tmp_str ) )
        {
            g_free ( tmp_str );
            break;
        }
        else if ( !g_utf8_collate ( g_utf8_casefold ( text, -1 ),
                                    g_utf8_casefold ( tmp_str, -1 ) ) )
        {
            g_free ( tmp_str );
            break;
        }

        g_free ( tmp_str );

        valid = gtk_tree_model_iter_next ( GTK_TREE_MODEL ( priv -> store ), &iter );
    }
    
    if ( valid )
        gtk_tree_store_remove ( priv -> store, &iter );
}


/**
* 
*
* \param combofix text
*
* \return
* */
void gtk_combofix_set_selection_callback ( GtkComboFix *combofix,
                        GCallback func,
                        gpointer data )
{
    GtkComboFixPrivate *priv = combofix -> priv;

    if ( func )
	    g_signal_connect ( G_OBJECT ( priv -> selection ),
                        "changed",
					    G_CALLBACK ( func ),
                        data );
}


/* *********************** the second part contains the construct object functions ******************************************** */

/**
* called when create a new combofix
*
* \param combofix
*
* \return
* */
static void gtk_combofix_class_init ( GtkComboFixClass *klass )
{
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = G_OBJECT_CLASS (klass);
    object_class -> dispose = gtk_combofix_dispose;
    object_class -> finalize = gtk_combofix_finalize;

    widget_class = GTK_WIDGET_CLASS (klass);

    g_type_class_add_private ( klass, sizeof ( GtkComboFixPrivate ) );
}


/**
* called when create a new combofix
*
* \param combofix
*
* \return
* */
static void gtk_combofix_init ( GtkComboFix *combofix )
{
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *frame;
    GtkCellRenderer *cell_renderer;
    GtkTreeViewColumn *tree_view_column;
    GtkWidget *scrolled_window;
    GtkComboFixPrivate *priv = GTK_COMBOFIX_GET_PRIVATE ( combofix );

    combofix -> priv = priv;

    /* set the fields of the combofix */
    priv -> force = FALSE;
    priv -> max_items = 0;
    priv -> case_sensitive = FALSE;
    priv -> visible_items = 0;

    /* the combofix is a vbox */
    vbox = gtk_vbox_new ( FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER ( combofix ), vbox );
    gtk_widget_show ( vbox );

    /* a hbox wich contains the entry and the button */
    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, TRUE, FALSE, 0 );
    gtk_widget_show ( hbox );

    /* set the entry */
    combofix -> entry = gtk_entry_new ( );
    g_signal_connect ( G_OBJECT ( combofix -> entry ),
                        "key-press-event",
                        G_CALLBACK ( gtk_combofix_key_press_event ),
                        combofix );
    g_signal_connect_object ( G_OBJECT ( combofix -> entry ),
                        "insert-text",
                        G_CALLBACK ( gtk_combofix_entry_insert ),
                        combofix,
                        G_CONNECT_AFTER | G_CONNECT_SWAPPED );
    g_signal_connect_object ( G_OBJECT ( combofix -> entry ),
                        "delete-text",
                        G_CALLBACK ( gtk_combofix_entry_delete ),
                        combofix,
                        G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    g_signal_connect_swapped ( G_OBJECT ( combofix -> entry ),
                        "expose-event",
                        G_CALLBACK ( gtk_combofix_expose_entry ),
                        combofix );
    g_signal_connect_after ( G_OBJECT ( combofix -> entry ),
                        "focus-in-event",
                        G_CALLBACK ( gtk_combofix_focus_in ),
                        combofix );

    g_signal_connect_after ( G_OBJECT ( combofix -> entry ),
                        "focus-out-event",
                        G_CALLBACK ( gtk_combofix_focus_out ),
                        combofix );
    gtk_box_pack_start ( GTK_BOX ( hbox ), combofix -> entry, TRUE, TRUE, 0 );
    gtk_widget_show ( combofix -> entry );

    /* set the button */
    priv -> button = gtk_button_new ( );
    gtk_button_set_relief ( GTK_BUTTON ( priv -> button ), GTK_RELIEF_NONE );
    gtk_container_add ( GTK_CONTAINER ( priv -> button ),
                        gtk_arrow_new ( GTK_ARROW_DOWN, GTK_SHADOW_NONE) );
    g_signal_connect_swapped ( G_OBJECT ( priv -> button ),
                        "clicked",
                        G_CALLBACK ( gtk_combofix_show_popup ),
                        combofix );
    gtk_box_pack_start ( GTK_BOX ( hbox ), priv -> button, FALSE, FALSE, 0 );
    gtk_widget_show_all ( priv -> button );

    /* set the popup but don't show it */
    priv -> popup = gtk_window_new ( GTK_WINDOW_POPUP );
    g_signal_connect ( G_OBJECT ( priv -> popup ),
                        "key-press-event",
                        G_CALLBACK (gtk_combofix_popup_key_press_event),
                        combofix );
    gtk_window_set_resizable ( GTK_WINDOW ( priv -> popup ), FALSE );

    g_signal_connect ( G_OBJECT ( priv -> popup ),
                        "button-press-event",
                        G_CALLBACK ( gtk_combofix_button_press ),
                        combofix );
    g_signal_connect ( G_OBJECT ( priv -> popup ),
                        "button-release-event",
                        G_CALLBACK ( gtk_combofix_button_release_event ),
                        combofix );

    frame = gtk_frame_new ( NULL );
    gtk_container_add ( GTK_CONTAINER ( priv -> popup ), frame );
    gtk_widget_show ( frame );

    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW(scrolled_window),
                        GTK_POLICY_AUTOMATIC,
                        GTK_POLICY_AUTOMATIC );
    gtk_container_add ( GTK_CONTAINER (frame), scrolled_window );
    gtk_widget_show (scrolled_window);

    /* Create the tree_store */
    priv -> store = gtk_tree_store_new ( COMBOFIX_N_COLUMNS,
                        G_TYPE_STRING,
                        G_TYPE_STRING,
                        G_TYPE_BOOLEAN,
                        G_TYPE_INT,
                        G_TYPE_BOOLEAN );

    /* we set the store in a filter to show only what is selected */
    priv -> model_filter = gtk_tree_model_filter_new ( GTK_TREE_MODEL ( priv -> store ), NULL );
    gtk_tree_model_filter_set_visible_column ( GTK_TREE_MODEL_FILTER ( priv -> model_filter ),
                        COMBOFIX_COL_VISIBLE );

    /* we set the filter in a sorting model */
    priv -> model_sort = gtk_tree_model_sort_new_with_model ( GTK_TREE_MODEL (
                        priv -> model_filter ) );
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE ( priv -> model_sort ),
                        COMBOFIX_COL_VISIBLE_STRING,
                        GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE ( priv -> model_sort ),
                        COMBOFIX_COL_VISIBLE_STRING,
                        (GtkTreeIterCompareFunc) gtk_combofix_default_sort_func,
                        combofix, NULL );

    /* make the column */
    cell_renderer = gtk_cell_renderer_text_new ();
    tree_view_column = gtk_tree_view_column_new_with_attributes ( "",
                                    cell_renderer,
                                    "text", COMBOFIX_COL_VISIBLE_STRING,
                                    NULL );
    gtk_tree_view_column_set_sizing ( tree_view_column,
                        GTK_TREE_VIEW_COLUMN_FIXED );

    /* set the sorting model in the tree view */
    priv -> tree_view = gtk_tree_view_new_with_model (
                                    GTK_TREE_MODEL ( priv -> model_sort ) );

    priv -> selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW ( priv -> tree_view ) );
    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION ( priv -> selection ),
                        GTK_SELECTION_SINGLE );
    gtk_tree_view_set_hover_selection ( GTK_TREE_VIEW ( priv -> tree_view ), TRUE );
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW ( priv -> tree_view ), FALSE );
    gtk_tree_view_append_column ( GTK_TREE_VIEW ( priv -> tree_view ), tree_view_column );
    gtk_tree_view_set_fixed_height_mode ( GTK_TREE_VIEW ( priv -> tree_view ), TRUE );
    gtk_tree_view_set_row_separator_func ( GTK_TREE_VIEW ( priv -> tree_view ),
                        (GtkTreeViewRowSeparatorFunc) gtk_combofix_separator_func,
                        combofix, NULL );

    g_signal_connect ( G_OBJECT ( priv -> tree_view ),
                        "button-press-event",
                        G_CALLBACK ( gtk_combofix_button_press_event ),
                        combofix );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), priv -> tree_view );

    gtk_widget_show ( priv -> tree_view );
}


/**
* called when destroy a combofix
*
* \param combofix
*
* \return
* */
static void gtk_combofix_dispose ( GObject *combofix )
{
    G_OBJECT_CLASS ( gtk_combofix_parent_class ) -> dispose ( combofix );
}


/**
* called when destroy combofix
*
* \param combofix
*
* \return
* */
static void gtk_combofix_finalize ( GObject *combofix )
{
    GtkComboFixPrivate *priv = GTK_COMBOFIX_GET_PRIVATE ( combofix );

    if ( priv -> old_entry && strlen ( priv -> old_entry ) )
        g_free ( priv -> old_entry );

    G_OBJECT_CLASS ( gtk_combofix_parent_class ) -> finalize ( combofix );
}


/* *********************** the third part contains all the static functions ******************************************** */

/**
* fill the model of the combofix given in param
* with the list given in param
* carreful : the list is not cleared, so if needed, must do it before
*
* \param                combofix
* \param list           a g_slist of strings
* \param list_number 	the number of the list for a complex, 0 else
*
* \return TRUE ok, FALSE pb
* */
static gboolean gtk_combofix_fill_store ( GtkComboFix *combofix,
                        GSList *list,
                        gint list_number )
{
    GSList *tmp_list;
    GtkTreeIter iter_parent;
    gchar *last_parent = NULL;
    GtkComboFixPrivate *priv = combofix -> priv;

    if ( !list )
	    return FALSE;

    /* normally the list cannot begin by a child, but we check here to
     * avoid a big crash */

    if ( list -> data && ( (gchar *) (list -> data))[0] == '\t' )
    {
        gboolean FAILED = TRUE;

        g_print ( "GtkComboFix error : the first entry in the list is a child, cannot fill the combofix\n" );
        g_return_val_if_fail ( FAILED, FALSE );
    }

    tmp_list = list;

    while (tmp_list)
    {
        gchar *string;
        gchar* tmp_str;

        string = tmp_list -> data;

        /* create the new iter where it's necessary and iter will focus on it */
        if (string)
        {
            if ( string[0] == '\t' )
            {
                /* it's a child */
                tmp_str = g_strconcat ( last_parent, " : ", string + 1, NULL );

                gtk_combofix_fill_iter_child ( priv -> store, &iter_parent, string + 1, tmp_str, list_number );

                g_free ( tmp_str );
            }
            else
            {
                /* it's a parent */
                gtk_combofix_fill_iter_parent ( priv -> store, &iter_parent, string, list_number );
                last_parent = string;
            }
        }

        tmp_list = tmp_list -> next;
    }

    priv -> visible_items += g_slist_length ( list );

    return TRUE;
}


/**
* fill a parent_iter of the model given in param
* with the string given in param
*
* \param store
* \param parent_iter
* \param string
* \param list_number 	the number of the list
*
* \return TRUE
* */
static gboolean gtk_combofix_fill_iter_parent ( GtkTreeStore *store,
                        GtkTreeIter *iter_parent,
                        const gchar *string,
                        gint list_number )
{
    gtk_tree_store_append ( store, iter_parent, NULL );
    gtk_tree_store_set ( store,
                iter_parent,
                COMBOFIX_COL_VISIBLE_STRING, string,
                COMBOFIX_COL_REAL_STRING, string,
                COMBOFIX_COL_VISIBLE, TRUE,
                COMBOFIX_COL_LIST_NUMBER, list_number,
                -1 );

    return TRUE;
}


/**
* fill a child_iter of the model given in param
* with the string given in param
*
* \param store
* \param parent_iter
* \param string
* \param list_number 	the number of the list
*
* \return TRUE
* */
static gboolean gtk_combofix_fill_iter_child ( GtkTreeStore *store,
                        GtkTreeIter *iter_parent,
                        const gchar *string,
                        const gchar *real_string,
                        gint list_number )
{
    GtkTreeIter iter_child;

    gtk_tree_store_append ( store, &iter_child, iter_parent );
    gtk_tree_store_set ( store,
                        &iter_child,
                        COMBOFIX_COL_VISIBLE_STRING, string,
                        COMBOFIX_COL_REAL_STRING, real_string,
                        COMBOFIX_COL_VISIBLE, TRUE,
                        COMBOFIX_COL_LIST_NUMBER, list_number,
                        -1 );

    return TRUE;
}


/**
 * called when insert in the entry of the combofix
 * look for a completion and show the according popup
 *
 * \param combofix
 *
 * \return TRUE to stop the signal, FALSE to continue
 * */
static gboolean gtk_combofix_entry_insert ( GtkComboFix *combofix )
{
    return gtk_combofix_entry_changed ( combofix, TRUE );
}


/**
 * called when insert in the entry of the combofix
 * look  show the according popup to the entry but don't complete the entry
 *
 * \param combofix
 *
 * \return TRUE to stop the signal, FALSE to continue
 * */
static gboolean gtk_combofix_entry_delete ( GtkComboFix *combofix )
{
    return gtk_combofix_entry_changed ( combofix, FALSE );
}


/**
 * called by the insert or delete function
 * update the popup according to the entry
 * set the completion only for insert
 *
 * \param combofix
 * \param insert_text TRUE when comes from an insert-text signal, FALSE for delete-text
 *
 * \return TRUE to stop the signal, FALSE to continue
 * */
static gboolean gtk_combofix_entry_changed ( GtkComboFix *combofix, gboolean insert_text )
{
    gchar *completed_string = NULL;
    const gchar *entry_string;
    GtkComboFixPrivate *priv = combofix -> priv;

    entry_string = gtk_entry_get_text ( GTK_ENTRY ( combofix -> entry ) );

    if ( strlen ( entry_string ) )
    {
        completed_string = gtk_combofix_update_visible_rows ( combofix,
                        entry_string);
        if ( completed_string == NULL )
            gtk_combofix_hide_popup ( combofix );
    }
    else if ( insert_text == 0 && strlen ( entry_string ) == 0 )
        gtk_combofix_hide_popup ( combofix );

    /* if force is set and there is no completed_string, we deleted 1 character by one
     * from the end to have again a completed string */
    if ( priv -> force
     &&
     !completed_string )
    {
        gchar *new_string = NULL;

        if ( entry_string && strlen ( entry_string) )
        {
            new_string = g_strdup ( entry_string );
            gtk_combofix_dialog ( _("You cannot create new payee or category "
                            "and sub-category without changing the options "
                            "in preferences"),
                            _("Warning you cannot create payee or category") );
        }

        while (!completed_string
               &&
               new_string
               &&
               strlen (new_string))
        {
            new_string[strlen (new_string) -1] = 0;
            if ( strlen ( new_string ) > 0 )
                completed_string = gtk_combofix_update_visible_rows ( combofix,
                                      new_string );
        }

        if ( completed_string )
        {
            gtk_combofix_set_text ( combofix, new_string );
            g_free ( new_string );
        }
        else
        {
            /* completed_string still NULL here means that even the first letter cannot
             * be set, so show all the list and erase the entry */
            gtk_combofix_set_text ( combofix, "" );
            gtk_combofix_set_all_visible_rows (combofix);

            return FALSE;
        }
    }

    if ( insert_text
     &&
     completed_string )
    {
        /* there is a completed_string, we set it in the entry only when inserting some text */
        gint position;

        position = gtk_editable_get_position ( GTK_EDITABLE ( combofix -> entry ) );
        gtk_combofix_set_text ( combofix, completed_string );
        gtk_editable_set_position ( GTK_EDITABLE ( combofix -> entry ), position );

        /* set the selection here doesn't work, so we will do it at the expose event */
        block_expose_event = 0;
    }

    /* show the popup */
    if ( priv -> visible_items && strlen ( entry_string )
     &&
     completed_string
     &&
     (!priv -> max_items
     ||
     priv -> visible_items < priv -> max_items ) )
    {
        gtk_combofix_set_popup_position ( combofix );
        gtk_widget_show ( priv -> popup );
        gtk_window_set_modal ( GTK_WINDOW ( priv -> popup ), TRUE );
    }

    return TRUE;
}


/**
 * called when the popup is exposed, used to set the selection
 * because don't work if set directly after the entry_set
 *
 * \param combofix
 *
 * \return FALSE
 * */
static gboolean gtk_combofix_expose_entry ( GtkComboFix *combofix )
{
    if ( block_expose_event )
	    return FALSE;

    block_expose_event = 1;

    gtk_editable_select_region ( GTK_EDITABLE (combofix -> entry),
				 gtk_editable_get_position (GTK_EDITABLE ( combofix -> entry) ),
				 -1 );
    return FALSE;
}


/**
 * set what is needed to be showed in the model according to the string given in param
 * return 	either the exact completion,
 * 		either the first completed string found according to the string parameter
 * the visible_items variable is set
 *
 * \param combofix
 * \param string the string wich will be completed
 *
 * \return the first completed string found
 * */
static gchar *gtk_combofix_update_visible_rows ( GtkComboFix *combofix,
                        const gchar *string )
{
    GtkTreeModel *model;
    gchar *complete_string = NULL;
    GtkTreePath *path;
    GtkTreeIter iter;
    gint path_ok;
    gint length;
    gboolean text_written = FALSE;
    gboolean separator = FALSE;
    GtkComboFixPrivate *priv = combofix -> priv;

    if (!combofix
	||
	!string )
	return NULL;

    length = strlen (string);
    if (!length)
	return NULL;

    priv -> visible_items = 0;
    model = GTK_TREE_MODEL ( priv -> store );
    path = gtk_tree_path_new_first ( );
    path_ok = gtk_tree_model_get_iter ( model, &iter, path );

    while ( path_ok )
    {
        gchar *model_string;
        gint show_row = 0;
        gint model_string_length;

        gtk_tree_model_get ( model,
                        &iter,
                        COMBOFIX_COL_REAL_STRING, &model_string,
                        COMBOFIX_COL_SEPARATOR, &separator,
                        -1 );

        /* The separators are never showed */
        if (separator)
            show_row = 0;
        else
        {
            model_string_length = strlen ( model_string );

            if ( priv -> case_sensitive )
            {
                show_row = !strncmp ( model_string,
                            string,
                            MIN ( length, model_string_length ) );
            }
            else
                show_row = !g_strncasecmp ( model_string,
                            string,
                            MIN ( length, model_string_length ) );

            if ( show_row )
            {
                /* if the current checked string is exactly the same as the wanted string,
                 * we keep it for completion, else we keep only the first approximation */
                if ( model_string_length == length )
                    complete_string = model_string;
                if ( !complete_string && model_string_length > length )
                    complete_string = model_string;

                priv -> visible_items ++;
                text_written = TRUE;
            }
        }
        gtk_tree_store_set ( GTK_TREE_STORE (model),
                        &iter,
                        COMBOFIX_COL_VISIBLE, show_row,
                        -1 );

        /* increment the path :
         * 	go to see the children only if the mother is showed */
        
        if ( gtk_tree_model_iter_has_child ( model, &iter )
             &&
             show_row )
            gtk_tree_path_down ( path );
        else
            gtk_tree_path_next (path);

        path_ok = gtk_tree_model_get_iter ( model, &iter, path );

        /* if path_ok is FALSE, perhaps we are on the end of the children list... */
        if ( !path_ok
            &&
            gtk_tree_path_get_depth ( path ) > 1 )
        {
            gtk_tree_path_up ( path );
            gtk_tree_path_next ( path );
            path_ok = gtk_tree_model_get_iter ( model, &iter, path );
        }
    }

    gtk_tree_path_free ( path );

    gtk_tree_view_expand_all ( GTK_TREE_VIEW ( priv -> tree_view ) );
    gtk_tree_selection_unselect_all ( priv -> selection );

    complete_string = gtk_combofix_get_first_string_from_sort_model ( priv -> model_sort, string );

    return complete_string;
}


/**
 * set all the rows of the list to be showed
 *
 * \param combofix
 *
 * \return FALSE
 * */
static gboolean gtk_combofix_set_all_visible_rows ( GtkComboFix *combofix )
{
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    gint path_ok;
    GtkComboFixPrivate *priv = combofix -> priv;

    if ( !combofix )
	    return FALSE;

    priv -> visible_items = 0;
    model = GTK_TREE_MODEL ( priv -> store);
    path = gtk_tree_path_new_first ( );
    path_ok = gtk_tree_model_get_iter ( model, &iter, path );

    while (path_ok)
    {
        gint value;

        /* if mixed_sort is set, we don't show any separator line */
        if ( priv -> mixed_sort )
        {
            gint separator;

            gtk_tree_model_get ( GTK_TREE_MODEL (model),
                     &iter,
                     COMBOFIX_COL_SEPARATOR, &separator,
                     -1 );

	    if (separator)
		    value = FALSE;
	    else
		    value = TRUE;
	}
	else
	    value = TRUE;

        gtk_tree_store_set ( GTK_TREE_STORE (model),
                     &iter,
                     COMBOFIX_COL_VISIBLE, value,
                     -1 );

        priv -> visible_items++;

        /* increment the path */
        if ( gtk_tree_model_iter_has_child ( model, &iter ) )
            gtk_tree_path_down ( path );
        else
            gtk_tree_path_next ( path );

        path_ok = gtk_tree_model_get_iter ( model, &iter, path );

        /* if path_ok is FALSE, perhaps we are on the end of the children list... */
        if (!path_ok
            &&
            gtk_tree_path_get_depth ( path ) > 1 )
        {
            gtk_tree_path_up ( path );
            gtk_tree_path_next ( path );
                path_ok = gtk_tree_model_get_iter ( model, &iter, path );
        }
    }
    gtk_tree_view_expand_all ( GTK_TREE_VIEW ( priv -> tree_view ) );

    return FALSE;
}


/**
 * set the position and the size of the popup
 *
 * \param combofix
 *
 * \return FALSE
 * */
static gboolean gtk_combofix_set_popup_position ( GtkComboFix *combofix )
{
    gint x, y;
    gint height;
    GdkRectangle rectangle;
    gint horizontal_separator;
    GtkComboFixPrivate *priv = combofix -> priv;

    if ( !combofix )
	    return FALSE;

    /* get the position of the combofix */
    gdk_window_get_origin ( GTK_WIDGET ( combofix->entry ) -> window, &x, &y );

    gtk_widget_style_get(GTK_WIDGET ( priv -> tree_view ),
			            "horizontal-separator", &horizontal_separator,
			            NULL );

    if ( GTK_WIDGET_REALIZED ( priv -> tree_view ) )
    {
        gtk_tree_view_get_cell_area ( GTK_TREE_VIEW ( priv -> tree_view),
                          gtk_tree_path_new_first ( ),
                          NULL,
                          &rectangle );
        /* the 4 is found at home, a good number to avoid the scrollbar with 1 item */
        height = ( priv -> visible_items ) * ( rectangle.height + horizontal_separator ) + 4;
    }
    else
        height = ( priv -> visible_items ) * ( GTK_WIDGET (
                        combofix -> entry ) -> allocation.height + horizontal_separator) + 4;

    /* if the popup is too small to contain all, we check to set it on the bottom or on the top
     * if the place on the top is more than 2 times bigger than the bottom, we set it on the top */
    if ( ( ( gdk_screen_height ( ) - y - GTK_WIDGET ( combofix ) -> allocation.height ) < height )
	 &&
	 ( ( ( gdk_screen_height () - y ) * 3 ) < y ) )
    {
        /* popup on the top */
        if ( y > height )
            y = y - height;
        else
        {
            height = y;
            y = 0;
        }
    }
    else
    {
        /* popup on the bottom */
        y += GTK_WIDGET ( combofix ) -> allocation.height;

        if ( ( gdk_screen_height ( ) - y ) < height )
            height = gdk_screen_height ( ) - y;
    }

    gtk_window_move ( GTK_WINDOW ( priv -> popup ), x, y );
    gtk_widget_set_size_request ( GTK_WIDGET ( priv -> popup ),
			            GTK_WIDGET ( combofix ) -> allocation.width,
			            height );

    return FALSE;
}



/**
 * called for a button press while the popup is showed
 * if the mouse is outside the popup, hide it
 *
 * \param popup
 * \param ev
 * \param combofix
 *
 * \return TRUE if we are on the popup, FALSE else
 * */
static gboolean gtk_combofix_button_press ( GtkWidget *popup,
                        GdkEventButton *ev,
                        GtkComboFix *combofix )
{
    if ( ( ev -> x_root > ( GTK_WIDGET (popup) -> allocation.x ))
	 &&
	 ( ev -> x_root < ( GTK_WIDGET (popup) -> allocation.x + GTK_WIDGET (popup) -> allocation. width ))
	 &&
	 ( ev -> y_root > ( GTK_WIDGET (popup) -> allocation.y ))
	 &&
	 ( ev -> x_root < ( GTK_WIDGET (popup) -> allocation.y +  GTK_WIDGET (popup) -> allocation. height)))
	return TRUE;

    gdk_pointer_ungrab ( GDK_CURRENT_TIME );
    gtk_widget_hide (popup);

    return FALSE;
}



/**
 * called when the entry receive a focus out event
 * hide the popup and check the content of the entry if force is set
 *
 * \param entry
 * \param ev
 * \param combofix
 *
 * \return FALSE
 * */
static gboolean gtk_combofix_focus_out ( GtkWidget *entry,
                        GdkEvent *ev,
                        GtkComboFix *combofix )
{
    gtk_combofix_hide_popup (combofix);

    /* hide the selection */
    gtk_editable_select_region ( GTK_EDITABLE (entry), 0, 0 );

    return ( FALSE );
}


/**
 * called when the entry receive a focus in event
 * 
 *
 * \param entry
 * \param ev
 * \param combofix
 *
 * \return FALSE
 * */
static gboolean gtk_combofix_focus_in ( GtkWidget *entry,
                        GdkEvent *ev,
                        GtkComboFix *combofix )
{
    const gchar *text;
    GtkComboFixPrivate *priv = GTK_COMBOFIX_GET_PRIVATE ( combofix );

    text = gtk_entry_get_text ( GTK_ENTRY ( combofix -> entry ) );

    if ( priv -> old_entry && strlen ( priv -> old_entry ) )
        g_free ( priv -> old_entry );

    if ( text && strlen ( text ) )
        priv -> old_entry = g_strdup ( text );
    else
        priv -> old_entry = NULL;

    return ( FALSE );
}


/**
 * the popup need to be modal to work fine, but the entry won't receive
 * some signal anymore... that function continue the signal to the entry
 *
 * \param popup
 * \param ev
 * \param combofix
 *
 * \return FALSE or TRUE according to the entry key press event return
 * */
static gboolean gtk_combofix_popup_key_press_event ( GtkWidget *popup,
                        GdkEventKey *ev,
                        GtkComboFix *combofix )
{
    gboolean return_val;

    g_signal_emit_by_name (combofix -> entry,
			   "key-press-event",
			   ev,
			   &return_val);
    return return_val;
}


/**
 * called for a key_press_event on the entry of the combofix
 *
 * \param entry
 * \param ev
 * \param combofix
 *
 * \return FALSE or TRUE, depends if need to block the signal
 * */
static gboolean gtk_combofix_key_press_event ( GtkWidget *entry,
                        GdkEventKey *ev,
                        GtkComboFix *combofix )
{
    GtkComboFixPrivate *priv = combofix -> priv;

    switch ( ev -> keyval )
    {
    case GDK_ISO_Left_Tab:
    case GDK_Tab :
    case GDK_KP_Enter :
    case GDK_Return :
        /* we get the current selection */
        if ( GTK_WIDGET_VISIBLE ( priv -> popup )
         &&
         strlen ( gtk_entry_get_text ( GTK_ENTRY ( combofix -> entry ) ) ) == 0 )
        {
           if ( ! gtk_combofix_choose_selection ( combofix ) )
            {
                /* here we did entry key, but no selection... so
                 * keep the current completion */
                gtk_combofix_hide_popup ( combofix );
                gtk_editable_select_region ( GTK_EDITABLE (combofix -> entry), 0, 0 );
            }
        }
        /* le traitement de ENTER est fait dans le formulaire */
        return FALSE;
        break;

    case GDK_Escape:
        if ( GTK_WIDGET_VISIBLE ( priv -> popup ))
        {
            gtk_combofix_hide_popup ( combofix );
            gtk_combofix_set_text ( combofix, priv -> old_entry );
            gtk_editable_select_region ( GTK_EDITABLE ( combofix -> entry ), 0, 0 );
            return TRUE;
        }
        break;

    case GDK_Down :
    case GDK_KP_Down :
        /* show the popup if necessary */
        if ( !GTK_WIDGET_VISIBLE ( priv -> popup ) )
            gtk_combofix_show_popup ( combofix );

        gtk_combofix_move_selection ( combofix, COMBOFIX_DOWN );
        gtk_combofix_choose_selection ( combofix );
        return TRUE;
        break;

    case GDK_Up :
    case GDK_KP_Up :
        /* move the selection up in the combofix only if the popup is showed,
         * else let the program works with the upper key */
        if (GTK_WIDGET_VISIBLE ( priv -> popup))
        {
            gtk_combofix_move_selection ( combofix, COMBOFIX_UP );
            gtk_combofix_choose_selection ( combofix );
            return TRUE;
        }
        break;

    case GDK_Page_Up :
    case GDK_KP_Page_Up :
        /* show the popup if necessary */
        if ( !GTK_WIDGET_VISIBLE ( priv -> popup ) )
            gtk_combofix_show_popup ( combofix );

        gtk_combofix_move_selection ( combofix, COMBOFIX_PAGE_UP );
        gtk_combofix_choose_selection ( combofix );
        return TRUE;
        break;

    case GDK_Page_Down :
    case GDK_KP_Page_Down :
        /* show the popup if necessary */
        if ( !GTK_WIDGET_VISIBLE ( priv -> popup ) )
            gtk_combofix_show_popup ( combofix );

        gtk_combofix_move_selection ( combofix, COMBOFIX_PAGE_DOWN );
        gtk_combofix_choose_selection ( combofix );
        return TRUE;
        break;
    }
    return FALSE;
}

/**
 * Called when a button press event is triggered on the tree view.
 * Select an entry if clicked.
 *
 * \param tree_view	GtkTreeView that triggered event.  It should be the tree view
			attached to a gtk combofix.
 * \param ev		Triggered event.
 * \param combofix	The GtkComboFix that contains tree view.
 *
 * \return TRUE to block the signal, FALSE else
 * */
static gboolean gtk_combofix_button_press_event ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        GtkComboFix *combofix )
{
    if (ev -> type ==  GDK_BUTTON_PRESS )
    {
        gtk_combofix_choose_selection (combofix);
        gtk_combofix_hide_popup ( combofix );
        return TRUE;
    }

    return FALSE;
}


/**
 * this function is very important, called when the popup
 * gets the release event. without that, when click outside,
 * move the mouse on the combofix will select some string with clicking anything
 *
 * to avoid that we need to propagate the signal release-event to the entry of the combofix
 *
 * \param popup
 * \param ev
 * \param combofix
 *
 * \return the returned value of the release event signal propagated
 * */
static gboolean gtk_combofix_button_release_event ( GtkWidget *popup,
                        GdkEventKey *ev,
                        GtkComboFix *combofix )
{
    gboolean return_val;

    g_signal_emit_by_name (combofix -> entry,
			   "button-release-event",
			   ev,
			   &return_val);
    return return_val;
}


/**
 * get the selected item and fill the entry with it
 *
 * \param combofix
 *
 * \return TRUE if ok, FALSE if no selection
 * */
static gboolean gtk_combofix_choose_selection ( GtkComboFix *combofix )
{
    GtkTreeIter iter;
    gchar *string;
    GtkComboFixPrivate *priv = combofix -> priv;

    /* if there is no selection, go away */
    if (!gtk_tree_selection_get_selected ( priv -> selection, NULL, &iter ) )
	    return FALSE;

    gtk_tree_model_get ( GTK_TREE_MODEL ( priv -> model_sort ),
			 &iter,
			 COMBOFIX_COL_REAL_STRING, &string,
			 -1 );

    if ( string && strlen ( string ) )
        gtk_combofix_set_text ( combofix, string );

    return TRUE;
}



/**
 * called to move the selection in the tree_view
 * didn't succeed to give the focus to the tree_view so must do
 * this manual
 *
 * \param combofix
 * \param direction a combofix_key_direction
 *
 * \return FALSE
 * */
static gboolean gtk_combofix_move_selection ( GtkComboFix *combofix,
                        gint direction )
{
    GtkTreeIter sorted_iter;
    gint result = 0;
    GtkComboFixPrivate *priv = combofix -> priv;

    if ( !combofix )
	    return FALSE;

    if ( gtk_tree_selection_get_selected ( priv -> selection, NULL, &sorted_iter ) )
    {
        /* there is already a selection */
        gint i;

        switch (direction)
        {
            case COMBOFIX_DOWN:
            result = gtk_combofix_move_selection_one_step ( combofix,
                                    &sorted_iter,
                                    COMBOFIX_DOWN );
            break;

            case COMBOFIX_UP:
            result = gtk_combofix_move_selection_one_step ( combofix,
                                    &sorted_iter,
                                    COMBOFIX_UP );
            break;

            case COMBOFIX_PAGE_DOWN:
            for ( i=0 ; i<gtk_combofix_get_rows_number_by_page ( combofix ) ; i++ )
                result = result | gtk_combofix_move_selection_one_step ( combofix,
                                             &sorted_iter,
                                             COMBOFIX_DOWN );
            break;

            case COMBOFIX_PAGE_UP:
            for ( i=0 ; i<gtk_combofix_get_rows_number_by_page ( combofix ) ; i++ )
                result = result | gtk_combofix_move_selection_one_step ( combofix,
                                             &sorted_iter,
                                             COMBOFIX_UP );
            break;
        }
    }
    else
    {
        /* there is no current selection,
         * get the first selectable line */
        gint separator = 0;
        do
        {
            if (separator)
            result = gtk_tree_model_iter_next ( GTK_TREE_MODEL ( priv -> model_sort ),
                                &sorted_iter);
            else
            result = gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( priv -> model_sort),
                                 &sorted_iter );
            if (result)
            gtk_tree_model_get ( GTK_TREE_MODEL ( priv -> model_sort),
                         &sorted_iter,
                         COMBOFIX_COL_SEPARATOR, &separator,
                         -1 );
            else
            separator = 0;
        }
        while (separator);
        }

        if (result)
        {
        GtkTreePath *path;

        gtk_tree_selection_select_iter ( priv -> selection, &sorted_iter );
        path = gtk_tree_model_get_path ( GTK_TREE_MODEL ( priv -> model_sort ),
                         &sorted_iter );
        if (path)
        {
            gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW ( priv -> tree_view),
                        path,
                        NULL,
                        FALSE,
                        0,
                        0 );
            gtk_tree_path_free ( path );
        }
    }

    return FALSE;
}


/**
 * called to select the text in the tree_view
 *
 * \param combofix
 * \param item name of the item
 *
 * \return FALSE
 * */
static gboolean gtk_combofix_select_item ( GtkComboFix *combofix,
                        const gchar *item )
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath *path;
    gchar *ptr;
    gchar *tmp_item = NULL;
    gint result = 0;
    GtkComboFixPrivate *priv = combofix -> priv;

    if ( !combofix )
	    return FALSE;
    if ( !item && strlen ( item ) == 0 )
        return FALSE;

    if ( ( ptr = g_utf8_strchr ( item, -1, ':' ) ) )
        tmp_item = g_strndup ( item, ( ptr - item ) -1 );
    else
        tmp_item = g_strdup ( item );
        
    model = GTK_TREE_MODEL ( priv -> model_sort );
    result = gtk_tree_model_get_iter_first ( model, &iter );

    while ( result )
    {
        gchar *tmp_str;
        
        gtk_tree_model_get ( model, &iter, COMBOFIX_COL_REAL_STRING, &tmp_str, -1 );

        if ( tmp_str
         &&
         tmp_item
         &&
         g_utf8_collate ( g_utf8_casefold ( tmp_str, -1 ),
         g_utf8_casefold ( tmp_item, -1 ) ) == 0 )
            break;

        result = gtk_tree_model_iter_next ( model, &iter);
	}

    g_free ( tmp_item );

    if ( result == 0 )
        result = gtk_tree_model_get_iter_first ( model, &iter );

    gtk_tree_selection_select_iter ( priv -> selection, &iter );
    path = gtk_tree_model_get_path ( model, &iter );
    if (path)
    {
        gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW ( priv -> tree_view ),
                        path,
                        NULL,
                        FALSE,
                        0,
                        0 );
        gtk_tree_path_free ( path );
    }

    return FALSE;
}


/**
 * return the number of visible rows showed on a page in the popup
 *
 * \param combofix
 *
 * \return the number of visible rows, 0 if problem
 * */
static gint gtk_combofix_get_rows_number_by_page ( GtkComboFix *combofix )
{
    gint return_value;
    GtkAdjustment *adjustment;
    GtkComboFixPrivate *priv = combofix -> priv;

    if (!combofix)
	return 0;

    adjustment = gtk_tree_view_get_vadjustment ( GTK_TREE_VIEW ( priv -> tree_view ) );
    return_value = priv -> visible_items * adjustment -> page_size / adjustment -> upper;

    return return_value;
}



/**
 * move the iter given in param of 1 step up or down and
 * go into the children if necessary
 *
 * \param model the tree model
 * \param iter a pointer to the iter to move
 * \param direction COMBOFIX_DOWN or COMBOFIX_UP
 *
 * \return TRUE ok, FALSE no change
 * */
static gboolean gtk_combofix_move_selection_one_step ( GtkComboFix *combofix,
                        GtkTreeIter *iter,
                        gint direction )
{
    gint result = 0;
    GtkTreePath *path;
    GtkTreePath *saved_path;
    GtkTreeModel *model;
    gint separator;
    GtkComboFixPrivate *priv = combofix -> priv;

    model = priv -> model_sort;
    path = gtk_tree_model_get_path ( model, iter );
    saved_path = gtk_tree_path_copy (path);

    switch (direction)
    {
	case COMBOFIX_DOWN:
	    do
	    {
		if ( gtk_tree_model_iter_has_child ( model, iter)
		     &&
		     gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( priv -> tree_view ), path ) )
		    gtk_tree_path_down ( path );
		else
		    gtk_tree_path_next ( path );

		result = gtk_tree_model_get_iter ( model, iter, path );

		/* if result is FALSE, perhaps we are on the end of the children list... */
		if ( !result
		    &&
		    gtk_tree_path_get_depth ( path ) > 1 )
		{
		    gtk_tree_path_up ( path );
		    gtk_tree_path_next ( path );
		    result = gtk_tree_model_get_iter ( model, iter, path );
		}

		/* check if we are not on a separator */
		if ( result )
		    gtk_tree_model_get ( model,
					    iter,
					    COMBOFIX_COL_SEPARATOR, &separator,
					    -1 );
		else
		    separator = 0;
	    }
	    while ( separator );
	    break;

	case COMBOFIX_UP:
	    do
	    {
		result = gtk_tree_path_prev ( path );

		if ( result )
		{
		    /* there is a prev path, but now, if we are on a parent, go to the last child,
		     * else, stay there */
		    result = gtk_tree_model_get_iter ( model, iter, path );

		    if ( result
			    &&
			    gtk_tree_model_iter_has_child ( model, iter )
			    &&
			    gtk_tree_view_row_expanded ( GTK_TREE_VIEW ( priv -> tree_view ), path ) )
		    {
			/* there is some children, go to the last one */
			gint i;

			gtk_tree_path_down ( path );

			for ( i = 0 ; i < gtk_tree_model_iter_n_children ( model, iter ) - 1 ; i++ )
			    gtk_tree_path_next ( path );

			result = gtk_tree_model_get_iter ( model, iter, path );
		    }
		}
		else
		{
		    /* there is no prev path, if we are not on the toplevel, go to the
		     * parent */

		    if ( gtk_tree_path_get_depth ( path ) > 1 )
		    {
                gtk_tree_path_up ( path );
                result = gtk_tree_model_get_iter ( model, iter, path );
		    }
		}
		/* check if we are not on a separator */
		if ( result )
		    gtk_tree_model_get ( model,
					    iter,
					    COMBOFIX_COL_SEPARATOR, &separator,
					    -1 );
		else
		    separator = 0;
	    }
	    while ( separator );
	    break;
    }

    gtk_tree_path_free ( path );

    /* if result is FALSE, iter was changed so set it to its initial value */
    if ( !result )
        gtk_tree_model_get_iter ( model, iter, saved_path );

    gtk_tree_path_free ( saved_path );

    return result;
}


/**
 * the default function to sort the combofix,
 * if mixed is set, all the list will be sorted by alphabetic order,
 * else, for a complex combofix, each list will be sorted by itself
 *
 * \param model_sort
 * \param iter_1
 * \param iter_2
 * \param combofix
 *
 * \return -1 if iter_1 before iter_2 ...
 * */
static gint gtk_combofix_default_sort_func ( GtkTreeModel *model_sort,
                        GtkTreeIter *iter_1,
                        GtkTreeIter *iter_2,
                        GtkComboFix *combofix )
{
    GtkComboFixPrivate *priv = combofix -> priv;
    gint list_number_1;
    gint list_number_2;
    gchar *string_1;
    gchar *string_2;
    gchar *cmp_string_1;
    gchar *cmp_string_2;
    gint return_value = 0;
    gboolean separator_1;
    gboolean separator_2;

    if ( iter_1 )
        gtk_tree_model_get ( GTK_TREE_MODEL (model_sort),
                        iter_1,
                        COMBOFIX_COL_LIST_NUMBER, &list_number_1,
                        COMBOFIX_COL_VISIBLE_STRING, &string_1,
                        COMBOFIX_COL_SEPARATOR, &separator_1,
                        -1 );
    else
        return -1;

    if ( iter_2 )
        gtk_tree_model_get ( GTK_TREE_MODEL (model_sort),
                        iter_2,
                        COMBOFIX_COL_LIST_NUMBER, &list_number_2,
                        COMBOFIX_COL_VISIBLE_STRING, &string_2,
                        COMBOFIX_COL_SEPARATOR, &separator_2,
                        -1 );
    else
        return 1;

    if ( priv -> mixed_sort == FALSE )
        return_value = list_number_1 - list_number_2;

    if ( return_value == 0 )
    {
        if ( separator_1 )
            return 1;
        if ( separator_2 )
            return -1;

        if ( string_1 == NULL )
            return -1;
        if ( string_2 == NULL )
            return 1;

        cmp_string_1 = g_utf8_collate_key ( string_1, -1 );
        cmp_string_2 = g_utf8_collate_key ( string_2, -1 );
        return_value = strcmp ( cmp_string_1, cmp_string_2 );

        g_free ( cmp_string_1 );
        g_free ( cmp_string_2 );
    }
    g_free ( string_1 );
    g_free ( string_2 );

    return return_value;
}


/**
 * check if the given row is or not a separator,
 * used in interne in gtk
 *
 * \param model
 * \param iter
 * \param combofix
 *
 * \return TRUE if it's a separator, FALSE else
 * */
static gboolean gtk_combofix_separator_func ( GtkTreeModel *model,
                        GtkTreeIter *iter,
                        GtkComboFix *combofix )
{
    gboolean value;

    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 iter,
			 COMBOFIX_COL_SEPARATOR, &value,
			 -1 );

    if (value)
	    return TRUE;
    return FALSE;
}


void gtk_combofix_dialog ( gchar *text, gchar *hint )
{
    GtkWidget *dialog;

    gchar *tmp_str;

    tmp_str = g_strconcat ( g_markup_printf_escaped (
                        "<span size=\"larger\" weight=\"bold\">%s</span>\n\n", hint ),
                        text, NULL );

    dialog = gtk_message_dialog_new ( NULL, 
                        GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
                        "%s", tmp_str );
    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG (dialog)->label ), tmp_str );

    g_free ( tmp_str );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy ( dialog );
}


/**
* vérifie si la chaine text existe déjà
*
* \param
*
* \return TRUE si trouvé FALSE autrement
* */
gboolean gtk_combofix_search_for_text (GtkTreeModel *model,
                        GtkTreePath *path,
                        GtkTreeIter *iter,
                        gpointer *data )
{
    gchar *tmp_str;
    gboolean case_sensitive;
    gboolean separator;
    gint return_value;

    gtk_tree_model_get ( GTK_TREE_MODEL( model ), iter,
			            COMBOFIX_COL_REAL_STRING, &tmp_str,
                        COMBOFIX_COL_SEPARATOR, &separator,
			            -1 );

    if ( separator )
    {
        g_free ( tmp_str );
        return FALSE;
    }

    case_sensitive = GPOINTER_TO_INT ( data[2] );
    if ( case_sensitive )
        return_value = !strcmp ( ( gchar *) data[0], tmp_str );
    else
        return_value = !g_utf8_collate ( g_utf8_casefold ( ( gchar *) data[0], -1 ),
                                         g_utf8_casefold ( tmp_str, -1 ) );
    if ( return_value )
        data[1] = GINT_TO_POINTER ( 1 );

    return return_value;
}


/**
 * returns the first occurrence of gtkcombofix
 *
 * \param sort_model
 * \param iter
 *
 * \return 
 */
gchar *gtk_combofix_get_first_string_from_sort_model ( GtkTreeModel *sort_model,
                        const gchar *string )
{
    GtkTreeIter iter;
    GtkTreeIter child_iter;
    gint length;
    gboolean separator = FALSE;
    gchar *model_string;
    gint model_string_length;

    length = strlen ( string );

    if ( gtk_tree_model_get_iter_first ( sort_model, &iter ) )
    {
        gtk_tree_model_get ( sort_model,
                        &iter,
                        COMBOFIX_COL_REAL_STRING, &model_string,
                        -1 );

        model_string_length = strlen ( model_string );

        if ( model_string_length >= length )
            return model_string;
        else if ( gtk_tree_model_iter_children ( sort_model, &child_iter, &iter ) )
        {
            gtk_tree_model_get ( sort_model,
                        &child_iter,
                        COMBOFIX_COL_REAL_STRING, &model_string,
                        COMBOFIX_COL_SEPARATOR, &separator,
                        -1 );
            return model_string;
        }
    }

    return g_strdup ( string );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
