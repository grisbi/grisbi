#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <gtk/gtk.h>

typedef enum 	_GsbAccountNotebookPages	GsbAccountNotebookPages;
typedef enum 	_GsbGeneralNotebookPages	GsbGeneralNotebookPages;
typedef struct	_struct_page				struct_page;
enum _GsbGeneralNotebookPages
{
    GSB_HOME_PAGE,
    GSB_ACCOUNT_PAGE,
    GSB_SCHEDULER_PAGE,
    GSB_PAYEES_PAGE,
    GSB_SIMULATOR_PAGE,
    GSB_CATEGORIES_PAGE,
    GSB_BUDGETARY_LINES_PAGE,
    GSB_REPORTS_PAGE,
    /** FIXME (later) : define an api so that plugin register here itself.  */
    GSB_AQBANKING_PAGE,
};

enum _GsbAccountNotebookPages
{
    GSB_TRANSACTIONS_PAGE,
    GSB_ESTIMATE_PAGE,
    GSB_HISTORICAL_PAGE,
    GSB_FINANCE_PAGE,
    GSB_PROPERTIES_PAGE,
};

/** \struct describe a page
 * */
struct _struct_page
{
    gint ordre;
    gint type_page;     /* GSB_GENERAL_NOTEBOOK_PAGES */
};

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
void gsb_gui_navigation_add_account ( gint account_number,
                        gboolean switch_to_account );
void gsb_gui_navigation_add_report ( gint report_number );
gboolean gsb_gui_navigation_check_scroll ( GtkWidget *tree_view,
                        GdkEventScroll *ev );
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
void gsb_gui_navigation_init_pages_list ( void );
void gsb_gui_navigation_init_tree_view ( void );
void gsb_gui_navigation_remove_account ( gint account_number );
void gsb_gui_navigation_remove_report ( gint report_number );
gboolean gsb_gui_navigation_select_line (GtkTreeSelection *selection,
										 GtkTreeModel *model );
gboolean gsb_gui_navigation_select_next ( void );
gboolean gsb_gui_navigation_select_prev ( void );
gboolean gsb_gui_navigation_set_page_list_order ( const gchar *order_list );
gboolean gsb_gui_navigation_set_selection ( gint page,
                        gint account_number,
                        gint report_number );
void gsb_gui_navigation_update_account ( gint account_number );
void gsb_gui_navigation_update_home_page ( void );
void gsb_gui_navigation_update_report ( gint report_number ) ;
void gsb_navigation_update_account_label ( gint account_number );
void gsb_navigation_update_statement_label ( gint account_number );
gboolean navigation_change_account ( gint new_account );
gboolean navigation_drag_data_received ( GtkTreeDragDest *drag_dest,
                        GtkTreePath *dest_path,
                        GtkSelectionData *selection_data );
gboolean navigation_row_drop_possible ( GtkTreeDragDest *drag_dest,
                        GtkTreePath *dest_path,
                        GtkSelectionData *selection_data );
gulong navigation_set_tree_view_selection_changed_signal (void);
/* END_DECLARATION */

#endif
