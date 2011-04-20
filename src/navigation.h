#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <gtk/gtk.h>

/** Holds data for the navigation tree.  */
enum navigation_cols { 
    NAVIGATION_PIX,
    NAVIGATION_PIX_VISIBLE,
    NAVIGATION_TEXT,
    NAVIGATION_FONT,
    NAVIGATION_PAGE,
    NAVIGATION_ACCOUNT,
    NAVIGATION_REPORT,
    NAVIGATION_SENSITIVE,
    NAVIGATION_ORDRE,        /* ordre des pages dans le modèle */
    NAVIGATION_TOTAL,
};


/** \struct describe a page
 * */
typedef struct
{
    gint ordre;
    gint type_page;     /* GSB_GENERAL_NOTEBOOK_PAGES */
} struct_page;


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void gsb_gui_navigation_add_account ( gint account_number,
                        gboolean switch_to_account );
void gsb_gui_navigation_add_report ( gint report_number );
void gsb_gui_navigation_create_account_list ( GtkTreeModel *model );
GtkWidget *gsb_gui_navigation_create_navigation_pane ( void );
void gsb_gui_navigation_free_pages_list ( void );
gint gsb_gui_navigation_get_current_account ( void );
gint gsb_gui_navigation_get_current_page ( void );
gint gsb_gui_navigation_get_current_report ( void );
gint gsb_gui_navigation_get_last_account ( void );
GtkTreeModel *gsb_gui_navigation_get_model ( void );
GQueue *gsb_gui_navigation_get_pages_list ( void );
GtkWidget *gsb_gui_navigation_get_tree_view ( void );
void gsb_gui_navigation_init_pages_list ( const gchar *order_list );
void gsb_gui_navigation_init_tree_view ( void );
void gsb_gui_navigation_remove_account ( gint account_number );
void gsb_gui_navigation_remove_report ( gint report_number );
gboolean gsb_gui_navigation_select_next ();
gboolean gsb_gui_navigation_select_prev ();
gboolean gsb_gui_navigation_set_page_list_order ( const gchar *order_list );
gboolean gsb_gui_navigation_set_selection ( gint page,
                        gint account_number,
                        gpointer report );
void gsb_gui_navigation_update_account ( gint account_number );
void gsb_gui_navigation_update_home_page ( void );
void gsb_gui_navigation_update_report ( gint report_number ) ;
void gsb_navigation_update_account_label ( gint account_number );
void gsb_navigation_update_statement_label ( gint account_number );
gboolean navigation_change_account ( gint *no_account );
gboolean navigation_drag_data_received ( GtkTreeDragDest *drag_dest,
                        GtkTreePath *dest_path,
                        GtkSelectionData *selection_data );
gboolean navigation_row_drop_possible ( GtkTreeDragDest *drag_dest, 
                        GtkTreePath *dest_path,
                        GtkSelectionData *selection_data );
/* END_DECLARATION */

#endif
