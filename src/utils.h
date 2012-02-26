#ifndef _UTILS_H
#define _UTILS_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean assert_account_loaded ();
GtkWidget *utils_combo_box_make_from_string_array ( gchar **array );
gboolean desensitive_widget ( gpointer object,
                        GtkWidget *widget );
gchar *get_gtk_run_version ( void );
void lance_mailer ( const gchar *uri );
gboolean lance_navigateur_web ( const gchar *uri );
gboolean met_en_normal ( GtkWidget *event_box,
                        GdkEventMotion *event,
                        gpointer pointeur );
gboolean met_en_prelight ( GtkWidget *event_box,
                        GdkEventMotion *event,
                        gpointer pointeur );
GtkWidget *new_paddingbox_with_title (GtkWidget *parent,
                        gboolean fill,
                        const gchar *title);
GtkWidget *new_vbox_with_title_and_icon ( gchar *title,
                        gchar *image_filename);
gboolean radio_set_active_linked_widgets ( GtkWidget *widget );
void register_button_as_linked ( GtkWidget *widget,
                        GtkWidget *linked );
gboolean sens_desensitive_pointeur ( GtkWidget *bouton,
                        GtkWidget *widget );
gboolean sensitive_widget ( gpointer object,
                        GtkWidget *widget );
void update_gui ( void );
void utils_gtk_combo_box_set_text_renderer ( GtkComboBox *combo,
                        gint num_col );
GtkListStore *utils_list_store_create_from_string_array ( gchar **array );
gboolean utils_tree_view_all_rows_are_selected ( GtkTreeView *tree_view );
void utils_set_tree_view_selection_and_text_color ( GtkWidget *tree_view );
gboolean utils_set_tree_view_background_color ( GtkWidget *tree_view,
                        gint color_column );
void utils_tree_view_set_expand_all_and_select_path_realize ( GtkWidget *tree_view,
                        const gchar *str_path );
void utils_ui_left_panel_add_line ( GtkTreeStore *tree_model,
                        GtkTreeIter *iter,
                        GtkWidget *notebook,
                        GtkWidget *child,
                        const gchar *title,
                        gint page );
gboolean utils_ui_left_panel_tree_view_selectable_func (GtkTreeSelection *selection,
                        GtkTreeModel *model,
                        GtkTreePath *path,
                        gboolean path_currently_selected,
                        gpointer data );
/* END_DECLARATION */
#endif
