#ifndef _GSB_DATA_PARTIAL_BALANCE_H
#define _GSB_DATA_PARTIAL_BALANCE_H (1)


/* START_INCLUDE_H */
#include "gsb_real.h"
#include "gsb_data_account.h"
/* END_INCLUDE_H */

/* START_DECLARATION */
gboolean gsb_data_partial_balance_drag_data_get ( GtkTreeDragSource * drag_source,
                        GtkTreePath * path,
                        GtkSelectionData * selection_data );
gboolean gsb_data_partial_balance_drag_data_received ( GtkTreeDragDest * drag_dest,
                        GtkTreePath * dest_path,
                        GtkSelectionData * selection_data );
gboolean gsb_data_partial_balance_get_colorise ( gint partial_balance_number );
gint gsb_data_partial_balance_get_currency ( gint partial_balance_number );
gsb_real gsb_data_partial_balance_get_current_amount ( gint partial_balance_number );
gchar *gsb_data_partial_balance_get_current_balance ( gint partial_balance_number );
kind_account gsb_data_partial_balance_get_kind ( gint partial_balance_number );
GSList *gsb_data_partial_balance_get_list ( void );
const gchar *gsb_data_partial_balance_get_liste_cptes ( gint partial_balance_number );
gchar *gsb_data_partial_balance_get_marked_balance ( gint partial_balance_number );
const gchar *gsb_data_partial_balance_get_name ( gint partial_balance_number );
gint gsb_data_partial_balance_get_number ( gpointer balance_ptr );
gboolean gsb_data_partial_balance_init_variables ( void );
gboolean gsb_data_partial_balance_set_colorise ( gint partial_balance_number,
                        gboolean colorise );
gboolean gsb_data_partial_balance_set_currency ( gint partial_balance_number,
                        gint currency );
gboolean gsb_data_partial_balance_set_kind ( gint partial_balance_number,
                        kind_account kind );
gboolean gsb_data_partial_balance_set_liste_cptes ( gint partial_balance_number,
                        const gchar *liste_cptes );
gboolean gsb_data_partial_balance_set_name ( gint partial_balance_number,
                        const gchar *name );
void gsb_partial_balance_add ( GtkWidget *button, GtkWidget *main_widget );
gboolean gsb_partial_balance_button_press ( GtkWidget *tree_view,
                        GdkEventButton *ev,
                        gpointer null );
void gsb_partial_balance_colorise_toggled ( GtkCellRendererToggle *cell,
                        gchar *path_str,
                        GtkWidget *tree_view );
GtkListStore *gsb_partial_balance_create_model ( void );
void gsb_partial_balance_delete ( GtkWidget *button, GtkWidget *main_widget );
void gsb_partial_balance_edit ( GtkWidget *button, GtkWidget *main_widget );
void gsb_partial_balance_fill_model ( GtkListStore *list_store );
gboolean gsb_partial_balance_key_press ( GtkWidget *tree_view, GdkEventKey *ev );
gint gsb_partial_balance_new_at_position ( const gchar *name, gint pos );
gboolean gsb_partial_balance_select_func ( GtkTreeSelection *selection,
                        GtkTreeModel *model,
                        GtkTreePath *path,
                        gboolean path_currently_selected,
                        GObject *main_widget );
/* END_DECLARATION */
#endif
