/* ComboFix Widget
 *
 *     Copyright (C)	2001-2006 Cédric Auger (cedric@grisbi.org) 
 *			2003-2006 Benjamin Drieu (bdrieu@april.org) 
 * 			http://www.grisbi.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gdk/gdkkeysyms.h>
#include <ctype.h>
#include "include.h"


/*START_INCLUDE*/
#include "gtk_combofix.h"
#include "./gsb_form_widget.h"
#include "./utils_str.h"
/*END_INCLUDE*/


/* Liste des fonctions statiques */

static void gtk_combofix_class_init ( GtkComboFixClass *klass );
static void gtk_combofix_init ( GtkComboFix *combofix );
static gboolean gtk_combofix_fill_store ( GtkComboFix *combofix,
					  GSList *list,
					  gint list_number );
static gboolean gtk_combofix_entry_insert ( GtkComboFix *combofix );
static gboolean gtk_combofix_entry_delete ( GtkComboFix *combofix );
static gboolean gtk_combofix_entry_changed ( GtkComboFix *combofix,
					     gboolean insert_text );
static gboolean gtk_combofix_show_popup ( GtkComboFix *combofix );
static gboolean gtk_combofix_expose_entry ( GtkComboFix *combofix );
static gchar *gtk_combofix_update_visible_rows ( GtkComboFix *combofix,
						 const gchar *string );
static gboolean gtk_combofix_set_all_visible_rows ( GtkComboFix *combofix );
static gboolean gtk_combofix_set_popup_position ( GtkComboFix *combofix );
static gboolean gtk_combofix_button_press ( GtkWidget *popup,
					    GdkEventButton *ev,
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
static gboolean gtk_combofix_choose_selection ( GtkComboFix *combofix );
static gboolean gtk_combofix_move_selection ( GtkComboFix *combofix,
					      gint direction );
static gboolean gtk_combofix_get_iter_model_from_tree_view ( GtkComboFix *combofix,
							     GtkTreeIter *iter_tree,
							     GtkTreeIter *iter_sort );
static gint gtk_combofix_get_rows_number_by_page ( GtkComboFix *combofix );
static gboolean gtk_combofix_move_selection_one_step ( GtkComboFix *combofix,
						       GtkTreeIter *iter,
						       gint direction );
static gboolean gtk_combofix_hide_popup ( GtkComboFix *combofix );
static gint gtk_combofix_default_sort_func ( GtkTreeModel *model_sort,
					     GtkTreeIter *iter_1,
					     GtkTreeIter *iter_2,
					     GtkComboFix *combofix );
static gboolean gtk_combofix_separator_func ( GtkTreeModel *model,
					      GtkTreeIter *iter,
					      GtkComboFix *combofix );




/* globals variables */
static gint block_expose_event;

enum combofix_columns {
    COMBOFIX_COL_VISIBLE_STRING = 0,
    COMBOFIX_COL_REAL_STRING,
    COMBOFIX_COL_VISIBLE,
    COMBOFIX_COL_LIST_NUMBER,
    COMBOFIX_COL_SEPARATOR,
};

enum combofix_key_direction {
    COMBOFIX_UP = 0,
    COMBOFIX_PAGE_UP,
    COMBOFIX_DOWN,
    COMBOFIX_PAGE_DOWN,
};

/*START_EXTERN*/
extern gint max;
extern GtkTreeSelection * selection;
extern GtkWidget *tree_view;
extern GtkWidget *window;
/*END_EXTERN*/


/* *********************** the first part contains all the extern functions ******************************************** */

guint gtk_combofix_get_type ( void )
{
    static guint gtk_combofix_type = 0;

    if ( !gtk_combofix_type )
    {
	static const GtkTypeInfo gtk_combofix_info = {
	    "GtkComboFix",
	    sizeof (GtkComboFix),
	    sizeof (GtkComboFixClass),
	    (GtkClassInitFunc) gtk_combofix_class_init,
	    (GtkObjectInitFunc) gtk_combofix_init,
	    NULL,
	    NULL,
	    (GtkClassInitFunc) NULL
	};

	gtk_combofix_type = gtk_type_unique ( gtk_hbox_get_type(),
					      &gtk_combofix_info );
    }

    return ( gtk_combofix_type );
}


/**
 * create a normal combofix, ie just 1 list
 * by default, force is not set, auto_sort is TRUE, no max items
 * and case unsensitive
 *
 * \param list a g_slist of name (\t at the begining makes it as a child)
 * \param force TRUE and the text must be in the list
 * \param sort TRUE and the list will be sorted automatickly
 * \param max_items the minimum of characters to show the popup
 * 
 * \return the new widget
 * */
GtkWidget *gtk_combofix_new ( GSList *list )
{
    GtkComboFix *combofix = GTK_COMBOFIX ( gtk_type_new ( gtk_combofix_get_type () ) );

    /* fill the field of the combofix */
    combofix -> force = FALSE;
    combofix -> complex = 0;
    combofix -> auto_sort = TRUE;
    combofix -> max_items = 0;
    combofix -> visible_items = 0;
    combofix -> case_sensitive = FALSE;

    gtk_combofix_fill_store ( combofix,
			      list,
			      0 );

    return ( GTK_WIDGET ( combofix ) );
}


/**
 * create a complex combofix, ie several list set one after the others
 * by default, force is not set, auto_sort is TRUE, no max items
 * and case unsensitive
 *
 * \param list a g_slist of name (\t at the begining makes it as a child)
 * \param force TRUE and the text must be in the list
 * \param sort TRUE and the list will be sorted automatickly
 * \param max_items the minimum of characters to show the popup
 * 
 * \return the new widget
 * */
GtkWidget *gtk_combofix_new_complex ( GSList *list )
{
    GSList *tmp_list;
    gint list_number = 0;
    gint length;

    GtkComboFix *combofix = GTK_COMBOFIX ( gtk_type_new ( gtk_combofix_get_type () ) );

    /* set the fields of the combofix */

    combofix -> force = FALSE;
    combofix -> complex = 1;
    combofix -> auto_sort = TRUE;
    combofix -> max_items = 0;
    combofix -> visible_items = 0;
    combofix -> case_sensitive = FALSE;

    tmp_list = list;
    length = g_slist_length (list);

    while ( tmp_list )
    {
	GtkTreeIter iter;

	gtk_combofix_fill_store ( combofix,
				  tmp_list -> data,
				  list_number );

	/* set the separator */
	if (list_number < (length-1))
	{
	    gtk_tree_store_append ( combofix -> store,
				    &iter,
				    NULL );
	    gtk_tree_store_set ( combofix -> store,
				 &iter,
				 COMBOFIX_COL_LIST_NUMBER, list_number,
				 COMBOFIX_COL_SEPARATOR, TRUE,
				 -1 );
	}

	list_number++;
	tmp_list = tmp_list -> next;
    }

    return ( GTK_WIDGET ( combofix ) );
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
void gtk_combofix_set_text ( GtkComboFix *combofix,
			     const gchar *text )
{
    g_return_if_fail (combofix);
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));
    g_return_if_fail ( text);

    g_signal_handlers_block_by_func ( G_OBJECT (combofix -> entry),
				      G_CALLBACK (gtk_combofix_entry_insert),
				      combofix );
    g_signal_handlers_block_by_func ( G_OBJECT (combofix -> entry),
				      G_CALLBACK (gtk_combofix_entry_delete),
				      combofix );
    gtk_entry_set_text ( GTK_ENTRY ( combofix -> entry ),
			 text );
    g_signal_handlers_unblock_by_func ( G_OBJECT (combofix -> entry),
					G_CALLBACK (gtk_combofix_entry_insert),
					combofix );
    g_signal_handlers_unblock_by_func ( G_OBJECT (combofix -> entry),
					G_CALLBACK (gtk_combofix_entry_delete),
					combofix );
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

    return ( gtk_entry_get_text ( GTK_ENTRY (combofix->entry)));
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
void gtk_combofix_set_force_text ( GtkComboFix *combofix,
				   gboolean value )
{
    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    combofix->force = value;
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
void gtk_combofix_set_max_items ( GtkComboFix *combofix,
				   gint max_items )
{
    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    combofix -> max_items = max_items;
}


/**
 * set if the list has to be automatickly sorted or not
 *
 * \param combofix
 * \param auto_sort TRUE for automatic sort
 *
 * \return
 * */
void gtk_combofix_set_sort ( GtkComboFix *combofix,
			     gboolean auto_sort )
{
    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    combofix -> auto_sort = auto_sort;
}

/**
 * set if the completion is case sensitive or not
 *
 * \param combofix
 * \param case_sensitive TRUE or FALSE
 *
 * \return
 * */
void gtk_combofix_set_case_sensitive ( GtkComboFix *combofix,
				       gboolean case_sensitive )
{
    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    combofix -> case_sensitive = case_sensitive;
}


/**
 * set the function of the enter key
 * either take the current selection and set it in the entry (FALSE)
 * either keep the current completion and close the popup (TRUE)
 *
 * \param combofix
 * \param enter_function TRUE or FALSE
 *
 * \return
 * */
void gtk_combofix_set_enter_function ( GtkComboFix *combofix,
				       gboolean  enter_function )
{
    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    combofix -> enter_function = enter_function;
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
void gtk_combofix_set_mixed_sort ( GtkComboFix *combofix,
				   gboolean mixed_sort )
{
    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    combofix -> mixed_sort = mixed_sort;
}





/**
 * show or hide the popup
 *
 * \param combofix
 * \param show TRUE to show the popup
 *
 * \return
 * */
void gtk_combofix_view_list ( GtkComboFix *combofix,
			      gboolean show )
{
    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    if (show)
	gtk_combofix_show_popup ( combofix );
    else
	gtk_combofix_hide_popup (combofix);
}


/**
 * change the list of an existing combofix
 *
 * \param combofix
 * \param list the new list
 *
 * \return TRUE if ok, FALSE if problem
 * */
gboolean gtk_combofix_set_list ( GtkComboFix *combofix,
				 GSList *list )
{
    g_return_val_if_fail (combofix, FALSE );
    g_return_val_if_fail (GTK_IS_COMBOFIX (combofix), FALSE);

    gtk_tree_store_clear (combofix -> store);

    if (combofix -> complex)
    {
	GSList *tmp_list;
	gint list_number = 0;
	gint length;

	tmp_list = list;
	length = g_slist_length (list);

	while ( tmp_list )
	{
	    GtkTreeIter iter;

	    gtk_combofix_fill_store ( combofix,
				      tmp_list -> data,
				      list_number );

	    /* set the separator */
	    if (list_number < (length-1))
	    {
		gtk_tree_store_append ( combofix -> store,
					&iter,
					NULL );
		gtk_tree_store_set ( combofix -> store,
				     &iter,
				     COMBOFIX_COL_LIST_NUMBER, list_number,
				     COMBOFIX_COL_SEPARATOR, TRUE,
				     -1 );
	    }
	    list_number++;
	    tmp_list = tmp_list -> next;
	}
    }
    else
    {
	gtk_combofix_fill_store ( combofix,
				  list,
				  0 );
    }
    return TRUE;
}




/* *********************** the second part contains all the static functions ******************************************** */


static void gtk_combofix_class_init ( GtkComboFixClass *klass )
{
    /*   GtkWidgetClass *widget_class; */
    /*   GtkObjectClass *gtk_object_class; */

    /*   widget_class = (GtkWidgetClass *)klass; */

    /*   gtk_object_class = (GtkObjectClass *)klass; */
    /*   gtk_object_class->destroy = gtk_combofix_destroy; */
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
    GtkWidget *button;
    GtkCellRenderer *cell_renderer;
    GtkTreeViewColumn *tree_view_column;
    GtkWidget *scrolled_window;

    /* the combofix is a vbox */
    vbox = gtk_vbox_new ( FALSE,
			  0 );
    gtk_container_add ( GTK_CONTAINER ( combofix ),
			vbox );
    gtk_widget_show ( vbox );

    /* a hbox wich contains the entry and the button */
    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );

    /* set the entry */
    combofix->entry = gtk_entry_new();
    g_signal_connect ( G_OBJECT (combofix -> entry),
		       "key-press-event",
		       G_CALLBACK ( gtk_combofix_key_press_event ),
		       combofix );
    g_signal_connect_object ( G_OBJECT (combofix -> entry),
			       "insert-text",
			       G_CALLBACK (gtk_combofix_entry_insert),
			       combofix,
			       G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    g_signal_connect_object ( G_OBJECT (combofix -> entry),
			       "delete-text",
			       G_CALLBACK (gtk_combofix_entry_delete),
			       combofix,
			       G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    g_signal_connect_swapped ( G_OBJECT (combofix -> entry),
			       "expose-event",
			       G_CALLBACK (gtk_combofix_expose_entry),
			       combofix );
    g_signal_connect_after ( G_OBJECT ( combofix->entry ),
			     "focus-out-event",
			     G_CALLBACK ( gtk_combofix_focus_out ),
			     combofix );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 combofix->entry,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( combofix->entry );

    /* set the button */
    button = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON (button),
			    GTK_RELIEF_NONE );
    gtk_container_add ( GTK_CONTAINER (button),
			gtk_arrow_new ( GTK_ARROW_DOWN,
					GTK_SHADOW_ETCHED_OUT) );
    g_signal_connect_swapped ( G_OBJECT (button),
			       "clicked",
			       G_CALLBACK ( gtk_combofix_show_popup ),
			       combofix );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 button,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all (button);

    /* set the popup but don't show it */
    combofix->popup = gtk_window_new ( GTK_WINDOW_POPUP );
    gtk_window_set_policy ( GTK_WINDOW ( combofix->popup ),
			    FALSE,
			    FALSE,
			    TRUE );
    g_signal_connect ( G_OBJECT ( combofix->popup ),
		       "button-press-event",
		       G_CALLBACK ( gtk_combofix_button_press ),
		       combofix );

    frame = gtk_frame_new ( NULL );
    gtk_container_add ( GTK_CONTAINER ( combofix -> popup ),
			frame );
    gtk_widget_show ( frame );

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW(scrolled_window),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_container_add ( GTK_CONTAINER (frame),
			scrolled_window );
    gtk_widget_show (scrolled_window);

    /* the tree_store is 4 columns :
     * COMBOFIX_COL_VISIBLE_STRING (a string) : what we see in the combofix
     * COMBOFIX_COL_REAL_STRING (a string) : what we set in the entry when selecting something
     * COMBOFIX_COL_VISIBLE (a boolean) : if that line has to be showed
     * COMBOFIX_COL_LIST_NUMBER (a int) : the number of the list for a complex combofix (0 else)
     * COMBOFIX_COL_SEPARATOR (a boolean) : TRUE for a separator
     * */
    combofix -> store = gtk_tree_store_new ( 5,
					     G_TYPE_STRING,
					     G_TYPE_STRING,
					     G_TYPE_BOOLEAN,
					     G_TYPE_INT,
					     G_TYPE_BOOLEAN );

    /* we set the store in a filter to show only what is selected */
    combofix -> model_filter = gtk_tree_model_filter_new ( GTK_TREE_MODEL (combofix -> store),
							   NULL );
    gtk_tree_model_filter_set_visible_column ( GTK_TREE_MODEL_FILTER (combofix -> model_filter),
					       COMBOFIX_COL_VISIBLE );

    /* we set the filter in a sorting model */
    combofix -> model_sort = gtk_tree_model_sort_new_with_model ( GTK_TREE_MODEL (combofix -> model_filter));
    gtk_tree_sortable_set_sort_column_id ( GTK_TREE_SORTABLE (combofix -> model_sort),
					   0, GTK_SORT_ASCENDING );
    gtk_tree_sortable_set_sort_func ( GTK_TREE_SORTABLE (combofix->model_sort),
				      0,
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
    combofix -> tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (combofix -> model_sort));
    gtk_tree_selection_set_mode ( GTK_TREE_SELECTION ( gtk_tree_view_get_selection ( GTK_TREE_VIEW(combofix -> tree_view))),
				  GTK_SELECTION_SINGLE );
    gtk_tree_view_set_hover_selection ( GTK_TREE_VIEW (combofix -> tree_view),
					TRUE );
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW (combofix -> tree_view),
					FALSE );
    gtk_tree_view_append_column ( GTK_TREE_VIEW (combofix -> tree_view),
				  tree_view_column );
    gtk_tree_view_set_fixed_height_mode ( GTK_TREE_VIEW(combofix -> tree_view),
					  TRUE );
    gtk_tree_view_set_row_separator_func ( GTK_TREE_VIEW(combofix -> tree_view),
					   (GtkTreeViewRowSeparatorFunc) gtk_combofix_separator_func,
					   combofix, NULL );
    g_signal_connect ( G_OBJECT (combofix -> tree_view),
		       "button-press-event",
		       G_CALLBACK (gtk_combofix_button_press_event),
		       combofix );
    gtk_container_add ( GTK_CONTAINER (scrolled_window),
			combofix -> tree_view);
    gtk_widget_show (combofix -> tree_view);
}



/**
 * fill the model of the combofix given in param
 * with the list given in param
 * carreful : the list is not cleared, so if needed, must do it before
 *
 * \param combofix
 * \param list a g_slist of strings
 * \param list_number the number of the list for a complex, 0 else
 *
 * \return TRUE ok, FALSE pb
 * */
static gboolean gtk_combofix_fill_store ( GtkComboFix *combofix,
					  GSList *list,
					  gint list_number )
{
    GSList *tmp_list;
    GtkTreeIter iter_parent;
    GtkTreeIter iter_child;
    gchar *last_parent = NULL;

    if ( !list )
	return FALSE;

    /* normally the list cannot begin by a child, but we check here to
     * avoid a big crash */

    if ( list -> data
	 &&
	 ((gchar *)(list -> data))[0] == '\t' )
    {
	printf ( "GtkComboFix error : the first entry in the list is a child, cannot fill the combofix\n");
	return FALSE;
    }

    tmp_list = list;

    while (tmp_list)
    {
	gchar *string;

	string = tmp_list -> data;

	/* create the new iter where it's necessary and iter will focus on it */
	if (string)
	{
	    if ( string[0] == '\t' )
	    {
		/* it's a child */
		gtk_tree_store_append ( combofix -> store,
					&iter_child,
					&iter_parent );
		gtk_tree_store_set ( combofix -> store,
				     &iter_child,
				     COMBOFIX_COL_VISIBLE_STRING, string + 1,
				     COMBOFIX_COL_REAL_STRING, g_strconcat ( last_parent,
									     " : ",
									     string + 1,
									     NULL ),
				     COMBOFIX_COL_VISIBLE, TRUE,
				     COMBOFIX_COL_LIST_NUMBER, list_number,
				     -1 );
	    }
	    else
	    {
		/* it's a parent */
		gtk_tree_store_append ( combofix -> store,
					&iter_parent,
					NULL );
		gtk_tree_store_set ( combofix -> store,
				     &iter_parent,
				     COMBOFIX_COL_VISIBLE_STRING, string,
				     COMBOFIX_COL_REAL_STRING, string,
				     COMBOFIX_COL_VISIBLE, TRUE,
				     COMBOFIX_COL_LIST_NUMBER, list_number,
				     -1 );
		last_parent = string;
	    }
	}
	tmp_list = tmp_list -> next;
    }

    combofix -> visible_items = combofix -> visible_items + g_slist_length (list);
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
    return gtk_combofix_entry_changed ( combofix,
					TRUE );
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
    return gtk_combofix_entry_changed ( combofix,
					FALSE );
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
static gboolean gtk_combofix_entry_changed ( GtkComboFix *combofix,
					     gboolean insert_text )
{
    gchar *completed_string = NULL;
    const gchar *entry_string;

    entry_string = gtk_entry_get_text ( GTK_ENTRY ( combofix -> entry ));

    gtk_editable_delete_selection ( GTK_EDITABLE (combofix -> entry));

    if (strlen (entry_string))
	completed_string = gtk_combofix_update_visible_rows ( combofix,
							      entry_string);
    else
	gtk_combofix_set_all_visible_rows (combofix);

    /* if force is set and there is no completed_string, we deleted 1 character by one
     * from the end to have again a completed string */
    if ( combofix -> force
	 &&
	 !completed_string )
    {
	gchar *new_string;

	new_string = my_strdup (entry_string);

	while (!completed_string
	       &&
	       new_string
	       &&
	       strlen (new_string))
	{
	    new_string[strlen (new_string) -1] = 0;
	    completed_string = gtk_combofix_update_visible_rows ( combofix,
								  new_string );
	}

	if (completed_string)
	{
	    gtk_combofix_set_text ( combofix,
				    new_string );
	    g_free ( new_string );

	    /* as we deleted something the user typed, we don't complete later, only
	     * show the popup */
	    completed_string = NULL;
	}
	else
	{
	    /* completed_string still NULL here means that even the first letter cannot
	     * be set, so show all the list and erase the entry */

	    gtk_combofix_set_text ( combofix,
				    "" );
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

	position = gtk_editable_get_position ( GTK_EDITABLE (combofix -> entry));
	gtk_combofix_set_text ( combofix,
				completed_string );
	gtk_editable_set_position ( GTK_EDITABLE (GTK_EDITABLE (combofix -> entry)),
				    position );

	/* set the selection here doesn't work, so we will do it at the expose event */
	block_expose_event = 0;
    }

    /* show the popup */
    if (combofix -> visible_items
	&&
	(!combofix -> max_items
	 ||
	 combofix -> visible_items < combofix -> max_items))
    {
	gtk_combofix_set_popup_position ( combofix );
	gtk_widget_show ( combofix -> popup );

	gdk_pointer_grab (combofix->popup->window, 
			  TRUE,
			  GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK,
			  NULL, 
			  NULL, 
			  GDK_CURRENT_TIME);
    }
    else
	gtk_combofix_hide_popup (combofix);

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
    if (block_expose_event
	||
	gsb_form_widget_check_empty (combofix -> entry))
	return FALSE;
    block_expose_event = 1;

    gtk_editable_select_region ( GTK_EDITABLE (combofix -> entry),
				 gtk_editable_get_position (GTK_EDITABLE (combofix -> entry)),
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
    gint separator_number = 0;
    gint root_element_numbers = 0;
    gint i;

    if (!combofix
	||
	!string )
	return NULL;

    length = strlen (string);
    if (!length)
	return NULL;

    combofix -> visible_items = 0;
    model = GTK_TREE_MODEL (combofix -> store);
    path = gtk_tree_path_new_first ();
    path_ok = gtk_tree_model_get_iter ( model,
					&iter,
					path );
    while (path_ok)
    {
	gchar *model_string;
	gint show_row;
	gint model_string_length;
	gint separator;

	gtk_tree_model_get ( model,
			     &iter,
			     COMBOFIX_COL_REAL_STRING, &model_string,
			     COMBOFIX_COL_SEPARATOR, &separator,
			     -1 );
	if (separator)
	{
	    show_row = 1;
	    separator_number++;
	}
	else
	{
	    model_string_length = strlen (model_string);

	    if ( combofix -> case_sensitive )
		show_row = !strncmp ( model_string,
				      string,
				      MIN (length, model_string_length));
	    else
		show_row = !g_strncasecmp ( model_string,
					    string,
					    MIN (length, model_string_length));

	    if (show_row)
	    {
		/* if the current checked string is exactly the same as the wanted string,
		 * we keep it for completion, else we keep only the first approximation */
		if (model_string_length == length)
		    complete_string = model_string;

		if (!complete_string
		    &&
		    model_string_length > length )
		    complete_string = model_string;
		combofix -> visible_items = combofix -> visible_items + 1;
	    }
	}
	gtk_tree_store_set ( GTK_TREE_STORE (model),
			     &iter,
			     COMBOFIX_COL_VISIBLE, show_row,
			     -1 );

	if ( gtk_tree_path_get_depth (path) == 1
	     &&
	     show_row )
	    root_element_numbers++;


	/* increment the path :
	 * 	go to see the children only if the mother is showed */
	if ( gtk_tree_model_iter_has_child ( model,
					     &iter)
	     &&
	     show_row )
	    gtk_tree_path_down (path);
	else
	    gtk_tree_path_next (path);

	path_ok = gtk_tree_model_get_iter ( model,
					    &iter,
					    path );

	/* if path_ok is FALSE, perhaps we are on the end of the children list... */
	if (!path_ok
	    &&
	    gtk_tree_path_get_depth (path) > 1)
	{
	    gtk_tree_path_up (path);
	    gtk_tree_path_next (path);
	    path_ok = gtk_tree_model_get_iter ( model,
						&iter,
						path );
	}
    }
    gtk_tree_path_free (path);

     /* we need now to set unvisible the separators at the end,
      * for that, go to the last path of the model_filter and
      * don't show all the separators at the end */

    path = gtk_tree_path_new_first ();
    for ( i=0 ; i<(root_element_numbers  - 1) ; i++ )
	gtk_tree_path_next (path);

    do
    {
	gint separator;
	path_ok = gtk_tree_model_get_iter ( GTK_TREE_MODEL (combofix -> model_sort),
					    &iter,
					    path );
	if (path_ok)
	{
	    gtk_tree_model_get ( GTK_TREE_MODEL (combofix -> model_sort),
				 &iter,
				 COMBOFIX_COL_SEPARATOR, &separator,
				 -1 );

	    if (separator)
	    {
		gtk_combofix_get_iter_model_from_tree_view ( combofix,
							     &iter,
							     &iter );
		gtk_tree_store_set ( GTK_TREE_STORE (combofix -> store),
				     &iter,
				     COMBOFIX_COL_VISIBLE, FALSE,
				     -1 );
		gtk_tree_path_prev (path);
	    }
	    else
		path_ok = 0;
	}
    }
    while (path_ok);

    gtk_tree_path_free (path);
    
    gtk_tree_view_expand_all ( GTK_TREE_VIEW (combofix -> tree_view));
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

    if (!combofix )
	return FALSE;

    combofix -> visible_items = 0;
    model = GTK_TREE_MODEL (combofix -> store);
    path = gtk_tree_path_new_first ();
    path_ok = gtk_tree_model_get_iter ( model,
					&iter,
					path );
    while (path_ok)
    {
	gint value;

	/* if mixed_sort is set, we don't show any separator line */
	if (combofix -> mixed_sort)
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
	combofix -> visible_items++;

	/* increment the path */
	if ( gtk_tree_model_iter_has_child ( model,
					     &iter))
	    gtk_tree_path_down (path);
	else
	    gtk_tree_path_next (path);

	path_ok = gtk_tree_model_get_iter ( model,
					    &iter,
					    path );

	/* if path_ok is FALSE, perhaps we are on the end of the children list... */
	if (!path_ok
	    &&
	    gtk_tree_path_get_depth (path) > 1)
	{
	    gtk_tree_path_up (path);
	    gtk_tree_path_next (path);
	    path_ok = gtk_tree_model_get_iter ( model,
						&iter,
						path );
	}
    }
    gtk_tree_view_expand_all ( GTK_TREE_VIEW (combofix -> tree_view));
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

    if (!combofix)
	return FALSE;

    /* get the position of the combofix */
    gdk_window_get_origin ( GTK_WIDGET ( combofix->entry )->window,
			    &x,
			    &y );

    gtk_widget_style_get(GTK_WIDGET(combofix -> tree_view),
			 "horizontal-separator", &horizontal_separator,
			 NULL);

    if (GTK_WIDGET_REALIZED (combofix -> tree_view))
    {
	gtk_tree_view_get_cell_area ( GTK_TREE_VIEW (combofix -> tree_view),
				      gtk_tree_path_new_first (),
				      NULL,
				      &rectangle );
	/* the 4 is found at home, a good number to avoid the scrollbar with 1 item */
	height = (combofix -> visible_items) * (rectangle.height + horizontal_separator) + 4;
    }
    else
	height = (combofix -> visible_items) * (GTK_WIDGET (combofix -> entry) -> allocation.height + horizontal_separator) + 4;

    /* if the popup is too small to contain all, we check to set it on the bottom or on the top
     * if the place on the top is more than 2 times bigger than the bottom, we set it on the top */
    if ( (( gdk_screen_height () - y - GTK_WIDGET ( combofix ) -> allocation.height ) < height )
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
	y = y + GTK_WIDGET ( combofix ) -> allocation.height;

	if ( ( gdk_screen_height () - y ) < height )
	    height = gdk_screen_height () - y;
    }

    gtk_widget_set_uposition ( GTK_WIDGET ( combofix->popup ),
			       x,
			       y );

    gtk_widget_set_usize ( GTK_WIDGET ( combofix->popup ),
			   GTK_WIDGET ( combofix ) ->allocation.width,
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

    /* check the entry if force is set */
    if ( combofix -> force )
	gtk_combofix_set_text ( combofix,
				gtk_combofix_update_visible_rows ( combofix,
								   gtk_entry_get_text (GTK_ENTRY (entry))));
    /* hide the selection */
    gtk_editable_select_region ( GTK_EDITABLE (entry),
				 0,
				 0 );
    return ( FALSE );
}



/**
 * hide the popup
 *
 * \param combofix
 *
 * \return FALSE
 * */
static gboolean gtk_combofix_hide_popup ( GtkComboFix *combofix )
{
    g_return_val_if_fail ( combofix != NULL, FALSE );
    g_return_val_if_fail ( GTK_IS_COMBOFIX (combofix), FALSE );

    if ( GTK_WIDGET_VISIBLE ( combofix -> popup ))
    {
	gdk_pointer_ungrab ( GDK_CURRENT_TIME );
	gtk_widget_hide ( combofix->popup );
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
static gboolean gtk_combofix_show_popup ( GtkComboFix *combofix )
{
    if ( GTK_WIDGET_VISIBLE (combofix -> popup))
	return FALSE;

    gtk_combofix_set_all_visible_rows (combofix);
    gtk_combofix_set_popup_position (combofix);
    gtk_widget_show ( combofix -> popup );
    gtk_widget_grab_focus ( GTK_WIDGET ( combofix -> entry ));

    return FALSE;
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
    switch ( ev -> keyval )
    {
	case GDK_KP_Enter :
	case GDK_Return :
	    if (combofix -> enter_function)
	    {
		/* we keep the current completion */
		gtk_combofix_hide_popup (combofix);
		gtk_editable_select_region ( GTK_EDITABLE (combofix -> entry),
					     0,
					     0 );
		return TRUE;
	    }
	    else
	    {
		/* we get the current selection */
		if ( GTK_WIDGET_VISIBLE ( combofix -> popup ))
		{
		    if (gtk_combofix_choose_selection (combofix))
			return TRUE;
		    else
		    {
			/* here we did entry key, but no selection... so
			 * keep the current completion */
			gtk_combofix_hide_popup (combofix);
			gtk_editable_select_region ( GTK_EDITABLE (combofix -> entry),
						     0,
						     0 );
		    }
		}
	    }

	case GDK_Escape:
	    if ( GTK_WIDGET_VISIBLE ( combofix -> popup ))
	    {
		gtk_combofix_hide_popup (combofix);
		gtk_editable_select_region ( GTK_EDITABLE (combofix -> entry),
					     0,
					     0 );
		return TRUE;
	    }
	    break;

	case GDK_Down :
	case GDK_KP_Down :
	    /* show the popup if necessary */
	    gtk_combofix_show_popup ( combofix );

	    gtk_combofix_move_selection ( combofix,
					  COMBOFIX_DOWN );
	    return TRUE;
	    break;

	case GDK_Up :
	case GDK_KP_Up :
	    gtk_combofix_move_selection ( combofix,
					  COMBOFIX_UP );
	    return TRUE;
	    break;

	case GDK_Page_Up :
	case GDK_KP_Page_Up :
	    gtk_combofix_move_selection ( combofix,
					  COMBOFIX_PAGE_UP );
	    return TRUE;
	    break;

	case GDK_Page_Down :
	case GDK_KP_Page_Down :
	    /* show the popup if necessary */
	    gtk_combofix_show_popup ( combofix );

	    gtk_combofix_move_selection ( combofix,
					  COMBOFIX_PAGE_DOWN );
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
	return TRUE;
    }

    return FALSE;
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
    GtkTreeSelection *tree_selection;
    gchar *string;

    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (combofix -> tree_view));
    
    /* if there is no selection, go away */
    if (!gtk_tree_selection_get_selected ( tree_selection,
					   NULL,
					   &iter ))
	return FALSE;
    
    gtk_tree_model_get ( GTK_TREE_MODEL (combofix -> model_sort),
			 &iter,
			 COMBOFIX_COL_REAL_STRING, &string,
			 -1 );
    gtk_combofix_set_text ( combofix,
			    string );
    gtk_combofix_hide_popup (combofix);
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
    GtkTreeSelection *tree_selection;
    gint result = 0;

    if (!combofix)
	return FALSE;

    tree_selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (combofix -> tree_view));
    
    if (gtk_tree_selection_get_selected ( tree_selection,
					  NULL,
					  &sorted_iter ))
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
		result = gtk_tree_model_iter_next ( GTK_TREE_MODEL (combofix -> model_sort),
						    &sorted_iter);
	    else
		result = gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (combofix -> model_sort),
							 &sorted_iter );
	    if (result)
		gtk_tree_model_get ( GTK_TREE_MODEL (combofix -> model_sort),
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

	gtk_tree_selection_select_iter ( tree_selection,
					 &sorted_iter );
	path = gtk_tree_model_get_path ( GTK_TREE_MODEL (combofix -> model_sort),
					 &sorted_iter );
	if (path)
	{
	    gtk_tree_view_scroll_to_cell ( GTK_TREE_VIEW (combofix -> tree_view),
					   path,
					   NULL,
					   FALSE,
					   0 , 0 );
	    gtk_tree_path_free (path);
	}
    }

    return FALSE;
}


/**
 * fill the iter of the model according to the iter of the tree_view
 * because theres is a filtering and sorting model between them
 *
 * \param combofix
 * \param iter_tree pointer to the tree iter, wich will be changed
 * \param iter_sort pointer to the sorting iter, wich we want to modify
 *
 * \return TRUE ok, FALSE problem
 * */
static gboolean gtk_combofix_get_iter_model_from_tree_view ( GtkComboFix *combofix,
							     GtkTreeIter *iter_tree,
							     GtkTreeIter *iter_sort )
{
    GtkTreeIter iter;

    if ( !combofix
	 ||
	 !iter_tree
	 ||
	 !iter_sort )
	return FALSE;

    gtk_tree_model_sort_convert_iter_to_child_iter ( GTK_TREE_MODEL_SORT (combofix -> model_sort),
						     &iter,
						     iter_sort );
    gtk_tree_model_filter_convert_iter_to_child_iter ( GTK_TREE_MODEL_FILTER (combofix -> model_filter),
						       iter_tree,
						       &iter );
    return TRUE;
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

    if (!combofix)
	return 0;

    adjustment = gtk_tree_view_get_vadjustment (GTK_TREE_VIEW (combofix -> tree_view));
    return_value = combofix -> visible_items * adjustment -> page_size / adjustment -> upper;

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

    model = combofix -> model_sort;
    path = gtk_tree_model_get_path ( model,
				     iter );
    saved_path = gtk_tree_path_copy (path);

    switch (direction)
    {
	case COMBOFIX_DOWN:
	    do
	    {
		if ( gtk_tree_model_iter_has_child ( model,
						     iter)
		     &&
		     gtk_tree_view_row_expanded ( GTK_TREE_VIEW (combofix -> tree_view),
						  path ))
		    gtk_tree_path_down (path);
		else
		    gtk_tree_path_next (path);

		result = gtk_tree_model_get_iter ( model,
						   iter,
						   path );

		/* if result is FALSE, perhaps we are on the end of the children list... */
		if (!result
		    &&
		    gtk_tree_path_get_depth (path) > 1)
		{
		    gtk_tree_path_up (path);
		    gtk_tree_path_next (path);
		    result = gtk_tree_model_get_iter ( model,
						       iter,
						       path );
		}

		/* check if we are not on a separator */
		if (result)
		    gtk_tree_model_get ( model,
					 iter,
					 COMBOFIX_COL_SEPARATOR, &separator,
					 -1 );
		else
		    separator = 0;
	    }
	    while (separator);
	    break;

	case COMBOFIX_UP:
	    do
	    {
		result = gtk_tree_path_prev (path);

		if (result)
		{
		    /* there is a prev path, but now, if we are on a parent, go to the last child,
		     * else, stay there */
		    result = gtk_tree_model_get_iter ( model,
						       iter,
						       path );

		    if ( result
			 &&
			 gtk_tree_model_iter_has_child ( model,
							 iter)
			 &&
			 gtk_tree_view_row_expanded ( GTK_TREE_VIEW (combofix -> tree_view),
						      path ))
		    {
			/* there is some children, go to the last one */
			gint i;

			gtk_tree_path_down (path);

			for ( i=0 ; i<gtk_tree_model_iter_n_children ( model, iter ) - 1 ; i++ )
			    gtk_tree_path_next (path);

			result = gtk_tree_model_get_iter ( model,
							   iter,
							   path );
		    }
		}
		else
		{
		    /* there is no prev path, if we are not on the toplevel, go to the
		     * parent */

		    if ( gtk_tree_path_get_depth (path) > 1)
		    {
			gtk_tree_path_up (path);
			result = gtk_tree_model_get_iter ( model,
							   iter,
							   path );
		    }
		}
		/* check if we are not on a separator */
		if (result)
		    gtk_tree_model_get ( model,
					 iter,
					 COMBOFIX_COL_SEPARATOR, &separator,
					 -1 );
		else
		    separator = 0;
	    }
	    while (separator);
	    break;
    }

    gtk_tree_path_free (path);

    /* if result is FALSE, iter was changed so set it to its initial value */
    if (!result)
	gtk_tree_model_get_iter ( model,
				  iter,
				  saved_path );
    gtk_tree_path_free (saved_path);

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
    gint list_number_1;
    gint list_number_2;
    gchar *string_1;
    gchar *string_2;
    gint return_value = 0;
    gboolean separator_1;
    gboolean separator_2;

    gtk_tree_model_get ( GTK_TREE_MODEL (model_sort),
			 iter_1,
			 COMBOFIX_COL_LIST_NUMBER, &list_number_1,
			 COMBOFIX_COL_VISIBLE_STRING, &string_1,
			 COMBOFIX_COL_SEPARATOR, &separator_1,
			 -1 );
    gtk_tree_model_get ( GTK_TREE_MODEL (model_sort),
			 iter_2,
			 COMBOFIX_COL_LIST_NUMBER, &list_number_2,
			 COMBOFIX_COL_VISIBLE_STRING, &string_2,
			 COMBOFIX_COL_SEPARATOR, &separator_2,
			 -1 );
    
    if (!combofix -> mixed_sort)
	return_value = list_number_1 - list_number_2;

    if (!return_value)
    {
	if (separator_1)
	    return_value = 1;
	else
	    if (separator_2)
		return_value = -1;
	    else
	    {
		gchar *cmp_string_1;
		gchar *cmp_string_2;

		cmp_string_1 = g_utf8_collate_key (string_1, -1);
		cmp_string_2 = g_utf8_collate_key (string_2, -1);
		return_value = g_ascii_strcasecmp ( cmp_string_1,
						    cmp_string_2 );
		g_free (cmp_string_1);
		g_free (cmp_string_2);
	    }
    }

    g_free (string_1);
    g_free (string_2);
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
    gint value;

    gtk_tree_model_get ( GTK_TREE_MODEL (model),
			 iter,
			 COMBOFIX_COL_SEPARATOR, &value,
			 -1 );

    if (value)
	return TRUE;
    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
